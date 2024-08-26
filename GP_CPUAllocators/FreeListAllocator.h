//
// Created by idemaj on 8/24/24.
//

#ifndef GREYPIXELENGINE_CPUWORK_FREELISTALLOCATOR_H
#define GREYPIXELENGINE_CPUWORK_FREELISTALLOCATOR_H

#include <cstdint>
#include <atomic>

namespace GP {
    // TODO: how do you make this fiber safe?

    struct FreeBlockDescriptor {
        FreeBlockDescriptor* pNext = nullptr;
    };

    struct FreeCell {
        FreeCell* pNext;
    };

    struct FreeListAllocator {
        uint32_t blockSize;
        uint32_t cellSize;
        FreeBlockDescriptor* pFirstBlock;
        FreeBlockDescriptor* pLastBlock;
        std::atomic<FreeCell*> pFreeCell;
    };

    FreeListAllocator* MakeFreeListAllocator(uint32_t blockSize, uint32_t cellSize);
    void DestroyFreeListAllocator(FreeListAllocator* allocator);

    void* AllocateFromList(FreeListAllocator* allocator);
    void FreeToList(FreeListAllocator* allocator, void* cellPtr);

} // GP

#endif //GREYPIXELENGINE_CPUWORK_FREELISTALLOCATOR_H
