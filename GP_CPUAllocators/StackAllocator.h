#ifndef GREYPIXELENGINE_CPUWORK_STACKALLOCATOR_H
#define GREYPIXELENGINE_CPUWORK_STACKALLOCATOR_H
#include <cstdint>

namespace GP {
    struct BlockDescriptor {
        // Taking the pointer of this plus the blockByteOffset returns the first byte after the last used byte of the block
        uint64_t blockByteOffset = sizeof(BlockDescriptor);
        // The next block in the allocator
        BlockDescriptor* pNext{};
    };

    struct AllocStack {
        BlockDescriptor* pFirstBlock{};
        BlockDescriptor* pCurrentBlock{};
    };

    struct StackAllocator {
        uint32_t numStacks;
        uint32_t blockByteSize;
        uint32_t pageSize;
        AllocStack* pStacks;
    };

    uint64_t GetPageByteSize();
    void* Allocate(StackAllocator* allocator, uint32_t threadID, uint32_t byteSize, uint32_t byteAlign=1);
    void ResetAllocator(StackAllocator* allocator);

    StackAllocator MakeThreadedStackAllocator(uint32_t numThreads, uint32_t numPagesPerBlock);
    void DestroyThreadedStackAllocator(StackAllocator* allocator);
}

#endif //GREYPIXELENGINE_CPUWORK_STACKALLOCATOR_H
