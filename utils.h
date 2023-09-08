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

#endif //MMAP_TESTS_UTILS_H
