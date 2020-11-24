cmake_minimum_required(VERSION 3.17)
project(Operating_Systems_ITMO_2020)

find_package(Threads REQUIRED)
set(CMAKE_CXX_STANDARD 14)

add_executable(lab1 "lab1/main.c" "lab1/memory_ops.c" "lab1/file_ops.c" "lab1/util.c")
target_link_libraries(lab1 PRIVATE Threads::Threads)
