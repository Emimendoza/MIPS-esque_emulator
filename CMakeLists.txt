cmake_minimum_required(VERSION 3.24)
project(MIPS)

set(CMAKE_CXX_STANDARD 20)

add_executable(MIPS main.cpp CPU.cpp CPU.h)
