cmake_minimum_required(VERSION 3.25)
project(DistributedKV)

set(CMAKE_CXX_STANDARD 11)
add_executable(DistributedKV main.cpp include/LRU.h
        include/KVDBHandler.h
        include/application.h
        include/sever.h)
include_directories(include)