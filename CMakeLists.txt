cmake_minimum_required(VERSION 3.10.0)
project(Xjson VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 可选：确保编译器支持 C++11
set(CMAKE_CXX_EXTENSIONS OFF)       # 可选：禁用编译器扩展（如 GNU 的 -std=gnu++11）

add_executable(Xjson main.cpp)

