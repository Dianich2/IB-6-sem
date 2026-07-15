cmake_minimum_required(VERSION 3.30)
project(task C)
add_executable(task task.c)
target_link_libraries(task m)