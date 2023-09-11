#include <atomic>
#include <thread>
#include <iostream>
#include <vector>
#include <random>

#include "utils.h"

int main(int argc, char **argv) {
    int thread_num = atoi(argv[1]);
    size_t count = atoll(argv[2]);
    size_t window_size = atoll(argv[3]);

    size_t elem_num = window_size / sizeof(uint8_t);

    std::cout << "==============================================================" << std::endl;
    std::cout << "start malloc rnd test:" << std::endl;
    std::cout << "\tthread_num = " << thread_num << std::endl;
    std::cout << "\tcount = " << count << std::endl;
    std::cout << "\twindow_size = " << window_size << std::endl;

    uint8_t *data = (uint8_t *) malloc(window_size);
    for (size_t i = 0; i < elem_num; ++i) {
        data[i] = i % 256;
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
