cmake_minimum_required(VERSION 3.15)
project(ThreadPool)

# SET(CXX_STANDARD_REQUIRED 11)

# message(STATUS "CXX STANDARD ${CXX_STANDARD}")

find_package (Threads)

include_directories(./)
add_library(ThreadPool ThreadPool.cpp)

add_executable(example example.cpp)
target_link_libraries(example ThreadPool Threads::Threads)
