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

int main(int argc, char** argv) {
    std::string filename = argv[1];
    int thread_num = atoi(argv[2]);
    bool seq = (atoi(argv[3]) == 1);
    int hint = atoi(argv[4]);
    int iter = atoi(argv[5]);
    size_t window_size = argc > 6 ? atoll(argv[6]) : std::numeric_limits<size_t>::max();
    int stride = argc > 7 ? atoi(argv[7]) : 1;

    size_t fs = file_size(filename);
    size_t elem_num = std::min(fs, window_size) / sizeof(uint8_t);
    size_t count = static_cast<size_t>(iter) * elem_num / static_cast<size_t>(stride);

    std::cout << "==============================================================" << std::endl;
    std::cout << "start mmap test: " << std::endl;
    std::cout << "\tthread_num = " << thread_num << std::endl;
    std::cout << (seq ? "\tsequential" : "\trandom") << std::endl;
    std::cout << (hint == 1 ? "\trandom hint" : (hint == 2 ? "\tsequential hint" : "\tnormal hint")) << std::endl;
    std::cout << "\titer = " << iter << ", count = " << count << std::endl;
    std::cout << "\twindow_size = " << window_size << std::endl;
    std::cout << "\tstride = " << stride << std::endl;

    int fd = open(filename.c_str(), O_RDONLY);
    uint8_t* data = (uint8_t*)mmap(NULL, fs, PROT_READ, MAP_SHARED, fd, 0);
    if (hint == 1) {
        madvise(data, fs, MADV_RANDOM);
    } else if (hint == 2) {
        madvise(data, fs, MADV_SEQUENTIAL);
    } else {
        madvise(data, fs, MADV_NORMAL);
    }

    warmup(data, elem_num, stride);

    std::vector<std::thread> threads;
    std::atomic<size_t> offset(0);
    std::atomic<uint64_t> ret(0);

    double start = get_current_time();
    for (int i = 0; i < thread_num; ++i) {
        threads.emplace_back([&]() {
            uint64_t local_ret = 0;
            const size_t iter_count = 32 * 1024;
            if (seq) {
                while (true) {
                    size_t cur_begin = std::min(offset.fetch_add(iter_count), count);
                    size_t cur_end = std::min(cur_begin + iter_count, count);
                    size_t cur_count = cur_end - cur_begin;
                    if (cur_count == 0) {
                        break;
                    }
                    size_t buffer_begin = (cur_begin * stride) % elem_num;
                    for (size_t i = 0; i < cur_count; ++i) {
                        local_ret += data[(buffer_begin + i * stride) % elem_num];
                    }
                }
            } else {
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
            }
            ret += local_ret;
        });
    }
    for (auto& thrd : threads) {
        thrd.join();
    }
    double end = get_current_time();

    std::cout << "elapsed: " << end - start << ", " << ret.load() << std::endl;
    std::cout << "access per second: " << static_cast<double>(count) / (end - start) << std::endl;

    return 0;
}
