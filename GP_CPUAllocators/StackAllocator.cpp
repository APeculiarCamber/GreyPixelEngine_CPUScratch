
#include "StackAllocator.h"
#include "PageAllocation.h"

#include <cassert>
#include <iostream>

namespace GP {

    StackAllocator MakeThreadedStackAllocator(uint32_t numThreads, uint32_t numPagesPerBlock) {
        uint32_t pageSize = GetPageByteSize();
        StackAllocator allocator {
                .numStacks = numThreads,
                .blockByteSize = numPagesPerBlock * pageSize,
                .pageSize = pageSize,
                .pStacks = new AllocStack[numThreads],
        };
        for (uint32_t t = 0; t < numThreads; t++) {
            auto* block = static_cast<BlockDescriptor*>(AllocateOnPageAlignment(allocator.pageSize, allocator.blockByteSize));
            *block = BlockDescriptor {
                    .blockByteOffset = sizeof(BlockDescriptor),
                    .pNext = nullptr,
            };
            allocator.pStacks[t].pFirstBlock = allocator.pStacks[t].pCurrentBlock = block;
        }
        return allocator;
    }

    uint32_t AlignTo(uint32_t start, uint32_t byteAlign) {
        return (start + (byteAlign - 1)) & ~(byteAlign - 1);
    }

    void *Allocate(StackAllocator *allocator, uint32_t threadID, uint32_t byteSize, uint32_t byteAlign) {
        assert(byteSize + AlignTo(sizeof(BlockDescriptor), byteAlign) <= allocator->blockByteSize);

        BlockDescriptor* currentBlock = allocator->pStacks[threadID].pCurrentBlock;
        uint32_t allocStart = AlignTo(currentBlock->blockByteOffset, byteAlign);
        uint32_t allocEnd = allocStart + byteSize;

        [[unlikely]] if (allocEnd > allocator->blockByteSize) {
            // Presumes that we clear but don't free the block memory, which is a reasonably fine assumption, not a leak for certain.
            if (currentBlock->pNext == nullptr) {
                currentBlock->pNext = (BlockDescriptor*)AllocateOnPageAlignment(allocator->pageSize, allocator->blockByteSize);
                *currentBlock->pNext = BlockDescriptor{ .blockByteOffset = sizeof(BlockDescriptor), .pNext = nullptr };
            }
            currentBlock = allocator->pStacks[threadID].pCurrentBlock = currentBlock->pNext;
            allocStart = AlignTo(currentBlock->blockByteOffset, byteAlign);
            allocEnd = allocStart + byteSize;
        }

        currentBlock->blockByteOffset = allocEnd;
        return ((uint8_t*)currentBlock) + allocStart;
    }

    void ResetAllocator(StackAllocator *allocator) {
        for (uint32_t t = 0; t < allocator->numStacks; t++) {
            BlockDescriptor* currentBlock = allocator->pStacks[t].pFirstBlock;
            allocator->pStacks[t].pCurrentBlock = currentBlock;
            while (currentBlock != nullptr) {
                currentBlock->blockByteOffset = sizeof(BlockDescriptor);
                currentBlock = currentBlock->pNext;
            }
        }
    }

    void DestroyThreadedStackAllocator(StackAllocator *allocator) {
        for (uint32_t t = 0; t < allocator->numStacks; t++) {
            BlockDescriptor* currentBlock = allocator->pStacks[t].pFirstBlock;
            while (currentBlock != nullptr) {
                BlockDescriptor* nextBlock = currentBlock->pNext;
                FreeOnPageAlignment(currentBlock);
                currentBlock = nextBlock;
            }
        }

        delete[] allocator->pStacks;
    }

} // GP