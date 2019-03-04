cmake_minimum_required(VERSION 3.10)
project(mystl)

set(CMAKE_CXX_STANDARD 14)

include_directories(deque)
include_directories(deque/data)
include_directories(map)
include_directories(map/data)
include_directories(map/data/five.memcheck)
include_directories(vector)
include_directories(vector/data/1000/data)

add_executable(map/data/)
