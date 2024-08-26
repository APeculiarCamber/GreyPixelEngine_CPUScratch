#include <iostream>
#include "GP_CPUAllocators/StackAllocator.h"
#include "GP_CPUAllocators/FreeListAllocator.h"

#include <thread>
#include <vector>
#include <sstream>

void MakeRandomListAllocs(GP::FreeListAllocator* allocator, std::chrono::time_point<std::chrono::steady_clock, std::chrono::steady_clock::duration> start) {
    auto id = std::hash<std::thread::id>{}(std::this_thread::get_id());
    for (int i = 0; i < 1000000; i++)
        *((size_t*)AllocateFromList(allocator)) = id;
    std::stringstream ss;
    auto end = std::chrono::steady_clock::now();
    ss << "Thread " << id << " finished in " << (end - start) / std::chrono::milliseconds(1) << "\n";
    std::cout << ss.str();
}


int main() {
    auto stackAllocator = GP::MakeThreadedStackAllocator(16, 16);
    size_t numInts = 65536 / (128 * sizeof(int));
    int* manyInts = static_cast<int *>(Allocate(&stackAllocator, 0, numInts * sizeof(int), 16));
    for (size_t i = 0; i < numInts; ++i) {
        manyInts[i] = i;
        std::cout << manyInts[i] << ", ";
    }
    std::cout << "\n";
    ResetAllocator(&stackAllocator);
    int* moreInts = static_cast<int *>(Allocate(&stackAllocator, 0, numInts * sizeof(int), 16));
    for (size_t i = 0; i < numInts; ++i) {
        std::cout << moreInts[i] << ", ";
    }
    std::cout << "\n";
    GP::DestroyThreadedStackAllocator(&stackAllocator);


    auto* freeAllocator = GP::MakeFreeListAllocator(GP::GetPageByteSize() * 32, 32);
    std::cout << std::endl;

    constexpr int threadCount = 16;
    std::vector<std::thread> threads;
    threads.reserve(threadCount);
    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < threadCount; i++) threads.emplace_back(MakeRandomListAllocs, freeAllocator, start);
    for (int i = 0; i < threadCount; i++) threads[i].join();

    GP::DestroyFreeListAllocator(freeAllocator);
}