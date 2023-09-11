#include <string>
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <algorithm>
#include <random>

#include <sys/mman.h>
#include <fcntl.h>

#include "utils.h"

int main(int argc, char **argv) {
    std::string filename = argv[1];
    int thread_num = atoi(argv[2]);
    int hint = atoi(argv[3]);
    size_t count = atoi(argv[4]);
    size_t window_size = argc > 5 ? atoll(argv[5]) : std::numeric_limits<size_t>::max();

    size_t fs = file_size(filename);
    size_t elem_num = std::min(fs, window_size) / sizeof(uint8_t);

    std::cout << "==============================================================" << std::endl;
    std::cout << "start mmap rnd test: " << std::endl;
    std::cout << "\tthread_num = " << thread_num << std::endl;
    std::cout << (hint == 1 ? "\trandom hint" : (hint == 2 ? "\tsequential hint" : "\tnormal hint")) << std::endl;
    std::cout << "\tcount = " << count << std::endl;
    std::cout << "\twindow_size = " << std::min(window_size, fs) << std::endl;

    int fd = open(filename.c_str(), O_RDONLY);
    uint8_t *data = (uint8_t *) mmap(NULL, fs, PROT_READ, MAP_SHARED, fd, 0);
    if (hint == 1) {
        madvise(data, fs, MADV_RANDOM);
    } else if (hint == 2) {
        madvise(data, fs, MADV_SEQUENTIAL);
    } else {
        madvise(data, fs, MADV_NORMAL);
    }

    warmup(data, elem_num, 1);

    std::vector<std::thread> threads;
    std::atomic<size_t> offset(0);
    std::atomic<uint64_t> ret(0);

    double start = get_current_time();
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back([&]() {
            uint64_t local_ret = 0;
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
                    local_ret += data[dis(gen)];
                    ++cur_begin;
                }
            }
            ret += local_ret;
        });
    }
    for (auto &thrd: threads) {
        thrd.join();
    }
    double end = get_current_time();

    std::cout << "elapsed: " << end - start << ", " << ret.load() << std::endl;
    std::cout << "access per second: " << static_cast<double>(count) / (end - start) << std::endl;

    return 0;
}
