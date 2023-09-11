#include <stdio.h>

#include <string>
#include <vector>

int main(int argc, char** argv) {
    std::string filename = argv[1];
    FILE* fout = fopen(filename.c_str(), "wb");
    size_t elem_num = atoll(argv[2]);

    const size_t chunk_size = 1024 * 1024;
    std::vector<uint8_t> chunk(chunk_size, 0);

    size_t chunk_num = elem_num / chunk_size;
    uint64_t offset = 0;
    for (size_t k = 0; k < chunk_size; ++k) {
        chunk[k] = k % 256;
    }
    for (size_t chunk_i = 0; chunk_i != chunk_num; ++chunk_i) {
        fwrite(chunk.data(), sizeof(uint8_t), chunk_size, fout);
    }

    size_t remaining = elem_num % chunk_size;
    fwrite(chunk.data(), sizeof(uint8_t), remaining, fout);

    return 0;
}
