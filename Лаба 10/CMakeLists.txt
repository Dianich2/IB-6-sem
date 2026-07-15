cmake_minimum_required(VERSION 3.30)
project(task C)
add_executable(task main.c)
target_link_libraries(task PRIVATE gmp)