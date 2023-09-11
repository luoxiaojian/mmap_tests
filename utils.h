#ifndef MMAP_TESTS_UTILS_H
#define MMAP_TESTS_UTILS_H

#include <string>

#include <sys/time.h>

inline size_t file_size(const std::string& filename) {
    FILE* fin = fopen(filename.c_str(), "rb");
    fseek(fin, 0, SEEK_END);
    size_t size = ftell(fin);
    fclose(fin);
    return size;
}

inline double get_current_time() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec * 1e-6;
}

inline void warmup(uint8_t* data, size_t size, int stride) {
    uint64_t sum = 0;
    double start = get_current_time();
    for (size_t i = 0; i < size; i += stride) {
        sum += data[i];
    }
    double end = get_current_time();
    std::cout << "warmup: " << end - start << ", " << sum << std::endl;
}

#endif //MMAP_TESTS_UTILS_H
