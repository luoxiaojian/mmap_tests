cmake_minimum_required(VERSION 3.25)
project(mmap_tests)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

add_executable(create_file create_file.cc)
add_executable(mmap_test mmap_test.cc)
target_link_libraries(mmap_test ${CMAKE_THREAD_LIBS_INIT})

