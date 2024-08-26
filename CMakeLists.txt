cmake_minimum_required(VERSION 3.29)
project(GreyPixelEngine_CPUWork)

set(CMAKE_CXX_STANDARD 23)

add_executable(GreyPixelEngine_CPUWork main.cpp
        GP_CPUAllocators/StackAllocator.cpp
        GP_CPUAllocators/StackAllocator.h
        GP_CPUAllocators/FreeListAllocator.cpp
        GP_CPUAllocators/FreeListAllocator.h
        GP_CPUAllocators/PageAllocation.cpp
        GP_CPUAllocators/PageAllocation.h)
