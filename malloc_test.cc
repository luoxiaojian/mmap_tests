#include <string>
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <random>

#include "utils.h"

int main(int argc, char** argv) {
    std::string filename = argv[1];
    int thread_num = atoi(argv[2]);
    bool seq = (atoi(argv[3]) == 1);
    size_t count = atoll(argv[4]);
    size_t window_size = atoll(argv[5]);
    int stride = atoi(argv[6]);

    std::cout << "start malloc test:" << std::endl;
    std::cout << "\tthread_num = " << thread_num << std::endl;
    std::cout << (seq ? "\tsequential" : "\trandom") << std::endl;
    std::cout << "\tcount = " << count << std::endl;
    std::cout << "\twindow_size = " << window_size << std::endl;
    std::cout << "\tstride = " << stride << std::endl;

    uint8_t* data = (uint8_t*)malloc(window_size);
    size_t elem_num = window_size / sizeof(uint8_t);
    for (size_t i = 0; i < elem_num; ++i) {
        data[i] = i % 256;
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