cmake_minimum_required(VERSION 3.28)
project(DistributedKV)

set(CMAKE_CXX_STANDARD 20)
add_executable(DistributedKV main.cpp include/LRU.h
        include/KVDBHandler.h
        include/Application.h)
include_directories(include)