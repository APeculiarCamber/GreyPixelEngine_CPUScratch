
#include "FreeListAllocator.h"
#include "PageAllocation.h"

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
#include <unistd.h>
#include <cstdlib>
#include <thread>
#include <iostream>
#include <sstream>

#endif


namespace GP {

    void* MakeFreeListBlock(uint32_t blockSize, uint32_t cellSize) {
        uint32_t pageSize = GetPageByteSize();
        void* block = AllocateOnPageAlignment(pageSize, blockSize);
        static_cast<FreeBlockDescriptor*>(block)->pNext = nullptr;
        uint32_t cellStep = cellSize;
        while (cellStep + (2*cellSize) < blockSize) {
            uint8_t* cell = static_cast<uint8_t*>(block) + cellStep;
            reinterpret_cast<FreeCell*>(cell)->pNext = reinterpret_cast<FreeCell*>(cell + cellSize);
            cellStep += cellSize;
        }
        uint8_t* cell = static_cast<uint8_t*>(block) + cellStep;
        reinterpret_cast<FreeCell*>(cell)->pNext = nullptr;
        return block;
    }

    FreeListAllocator* MakeFreeListAllocator(uint32_t blockSize, uint32_t cellSize) {
        cellSize = NextPowerOfTwo(cellSize);
        FreeListAllocator* allocator = new FreeListAllocator {
            .blockSize = blockSize,
            .cellSize = cellSize,
            .pFirstBlock = static_cast<FreeBlockDescriptor *>(MakeFreeListBlock(blockSize, cellSize)),
        };
        allocator->pLastBlock = allocator->pFirstBlock;
        allocator->pFreeCell.store(reinterpret_cast<FreeCell*>(reinterpret_cast<uint8_t*>(allocator->pFirstBlock) + cellSize), std::memory_order_relaxed);
        return allocator;
    }

    void DestroyFreeListAllocator(FreeListAllocator *allocator) {
        FreeBlockDescriptor* currentBlock = allocator->pFirstBlock;
        while (currentBlock != nullptr) {
            FreeBlockDescriptor* nextBlock = currentBlock->pNext;
            FreeOnPageAlignment(currentBlock);
            currentBlock = nextBlock;
        }
        delete allocator;
    }

    // The cell pointer will never be this, unless cell size is 0 or 1, which is not a valid cell size
#define RESERVE_ALLOC_PTR_VAL (FreeCell*)1
    FreeCell* HandleBlockAllocForThreads(FreeListAllocator* allocator) {
        FreeCell* returnCell = nullptr;
        bool gotContract = allocator->pFreeCell.compare_exchange_weak(returnCell, RESERVE_ALLOC_PTR_VAL,
                                                        std::memory_order_release, std::memory_order_relaxed);
        if (not gotContract) {
            while (returnCell == RESERVE_ALLOC_PTR_VAL) returnCell = allocator->pFreeCell.load();
            return returnCell;
        } else {
            auto* block = static_cast<FreeBlockDescriptor *>(MakeFreeListBlock(allocator->blockSize, allocator->cellSize));
            allocator->pLastBlock->pNext = block;
            allocator->pLastBlock = block;
            allocator->pFreeCell.store(reinterpret_cast<FreeCell*>(reinterpret_cast<uint8_t*>(allocator->pLastBlock) + allocator->cellSize));
            return allocator->pFreeCell;
        }
    }

    void *AllocateFromList(FreeListAllocator *allocator) {
        FreeCell* returnCell, *nextCell;
        returnCell = allocator->pFreeCell.load(std::memory_order_acquire);
        do {
            while (returnCell == nullptr or returnCell == RESERVE_ALLOC_PTR_VAL) {
                returnCell = HandleBlockAllocForThreads(allocator);
            }
            nextCell = returnCell->pNext;
        } while (not allocator->pFreeCell.compare_exchange_weak(returnCell, nextCell,
                                                                std::memory_order_release, std::memory_order_relaxed));
        return returnCell;
    }

    void FreeToList(FreeListAllocator *allocator, void *cellPtr) {
        FreeCell* cell = static_cast<FreeCell*>(cellPtr);
        cell->pNext = allocator->pFreeCell;
        allocator->pFreeCell.store(cell, std::memory_order_relaxed);
    }

} // GP