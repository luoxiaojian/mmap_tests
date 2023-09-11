cmake_minimum_required(VERSION 3.16)
project(mmap_tests)

set(CMAKE_CXX_STANDARD 17)
set(DEFAULT_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -g")

set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
find_package(Threads REQUIRED)

add_executable(create_file create_file.cc)
add_executable(mmap_test mmap_test.cc)
target_link_libraries(mmap_test ${CMAKE_THREAD_LIBS_INIT})
add_executable(malloc_test malloc_test.cc)
target_link_libraries(malloc_test ${CMAKE_THREAD_LIBS_INIT})

add_executable(mmap_rnd_test mmap_rnd_test.cc)
target_link_libraries(mmap_rnd_test ${CMAKE_THREAD_LIBS_INIT})
add_executable(malloc_rnd_test malloc_rnd_test.cc)
target_link_libraries(malloc_rnd_test ${CMAKE_THREAD_LIBS_INIT})
