#include <string>
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils.h"
#include "buffer_pool.h"

#define VERIFY

void warmup_bp(BufferPool& bp, size_t size) {
  uint64_t sum = 0;
  double start = get_current_time();
  for (size_t k = 0; k < size; ++k) {
    sum += bp.get_byte(k);
  }
  double end = get_current_time();
  std::cout << "warmup: " << end - start << ", " << sum << std::endl;
}

int main(int argc, char **argv) {
    std::string filename = argv[1];
    int thread_num = atoi(argv[2]);
    size_t count = atoi(argv[3]);
    size_t window_size = argc > 4 ? atoll(argv[4]) : std::numeric_limits<size_t>::max();
    int wu = argc > 5 ? atoi(argv[5]) : 0;

    size_t fs = file_size(filename);
    size_t elem_num = std::min(fs, window_size) / sizeof(uint8_t);

    std::cout << "==============================================================" << std::endl;
    std::cout << "start buffer_pool rnd test: " << std::endl;
    std::cout << "\tthread_num = " << thread_num << std::endl;
    std::cout << "\tcount = " << count << std::endl;
    std::cout << "\twindow_size = " << std::min(window_size, fs) << std::endl;

    BufferPool bp(filename, 68719476736ull);

    if (wu) {
      warmup_bp(bp, elem_num);
    }

    std::vector<std::thread> threads;
    std::atomic<size_t> offset(0);
    std::atomic<uint64_t> ret(0);
#ifdef VERIFY
    std::atomic<uint64_t> correct(0);
#endif

    double start = get_current_time();
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back([&]() {
            uint64_t local_ret = 0;
	    uint8_t cur;
#ifdef VERIFY
	    uint64_t local_correct = 0;
#endif
            const size_t iter_count = 32 * 1024;
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<uint64_t> dis(0, elem_num);
            while (true) {
                size_t cur_begin = std::min(offset.fetch_add(iter_count), count);
                size_t cur_end = std::min(cur_begin + iter_count, count);
                if (cur_begin == cur_end) {
                    break;
                }
                while (cur_begin != cur_end) {
                    uint64_t pos = dis(gen);
		    cur = bp.get_byte(pos);
                    local_ret += cur;
#ifdef VERIFY
		    local_correct += (pos & 255);
#endif
                    ++cur_begin;
                }
            }
#ifdef VERIFY
	    correct += local_correct;
#endif
            ret += local_ret;
        });
    }
    for (auto &thrd: threads) {
        thrd.join();
    }
    double end = get_current_time();

#ifdef VERIFY
    std::cout << "elapsed: " << end - start << ", " << ret.load() << ", " << correct.load() << std::endl;
#else
    std::cout << "elapsed: " << end - start << ", " << ret.load() << std::endl;
#endif
    std::cout << "access per second: " << static_cast<double>(count) / (end - start) << std::endl;

    return 0;
}
