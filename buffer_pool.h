#ifndef MMAP_TESTS_BUFFER_POOL_H
#define MMAP_TESTS_BUFFER_POOL_H

#include <array>
#include <random>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#include "utils.h"

#define PAGE_SIZE 4096

class BufferPool {
 public:
  BufferPool(const std::string& filename, size_t buffer_size) {
    fd_ = open(filename.c_str(), O_RDONLY);
    file_size_ = file_size(filename);
    entry_num_ = (buffer_size + PAGE_SIZE - 1) / PAGE_SIZE;
    block_table_.resize((file_size_ + PAGE_SIZE - 1) / PAGE_SIZE, entry_num_);
    blocks_entries_.resize(entry_num_);
    block_stat_.resize(entry_num_, std::make_pair(0, 0));
  }

  uint8_t get_byte(size_t offset) {
    size_t block_id = offset / PAGE_SIZE;
    size_t block_offset = offset % PAGE_SIZE;

    uint8_t ret;
    access_page(block_id, [&](const std::array<uint8_t, 4096>& block) {
      ret = block[block_offset];
    });
    return ret;
  }

 private:
  uint32_t get_idle_page() {
    static thread_local std::random_device rd;
    static thread_local std::mt19937 gen(rd());
    static thread_local std::uniform_int_distribution<uint32_t> dis(
        0, entry_num_ - 1);

    while (true) {
      uint32_t entry_id = dis(gen);
      static constexpr int32_t INVALID_FLAG =
          std::numeric_limits<int32_t>::min();
      if (__sync_bool_compare_and_swap(&block_stat_[entry_id].first, 0,
                                       INVALID_FLAG)) {
        return entry_id;
      }
    }
  }

  template <typename FUNC_T>
  void access_page(size_t page_id, FUNC_T func) {
    uint32_t entry_id = block_table_[page_id];
    if (entry_id < entry_num_) {
      int reader_count = __sync_add_and_fetch(&block_stat_[entry_id].first, 1);
      if (reader_count > 0) {
        if (block_stat_[entry_id].second == page_id) {
          func(blocks_entries_[entry_id]);
          return;
        } else {
          __sync_sub_and_fetch(&block_stat_[entry_id].first, 1);
        }
      }
    }

    uint32_t idle_entry_id = get_idle_page();
    pread(fd_, blocks_entries_[idle_entry_id].data(), PAGE_SIZE,
          page_id * PAGE_SIZE);
    block_stat_[idle_entry_id].second = page_id;
    block_stat_[idle_entry_id].first = 1;
    func(blocks_entries_[idle_entry_id]);
    __sync_bool_compare_and_swap(&block_table_[page_id], entry_id,
                                 idle_entry_id);
    __sync_sub_and_fetch(&block_stat_[idle_entry_id].first, 1);
  }

  int fd_;
  size_t file_size_;
  uint32_t entry_num_;

  std::vector<uint32_t> block_table_;
  std::vector<std::array<uint8_t, 4096>> blocks_entries_;
  std::vector<std::pair<int32_t, size_t>> block_stat_;
};

#endif  // MMAP_TESTS_BUFFER_POOL_H
