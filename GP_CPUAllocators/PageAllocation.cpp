//
// Created by idemaj on 8/25/24.
//

#include "PageAllocation.h"

namespace GP {
    uint32_t NextPowerOfTwo(uint32_t n) {
        n--;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        return n + 1;
    }

    uint64_t GetPageByteSize() {
#if defined(_WIN32) || defined(_WIN64)
        SYSTEM_INFO si;
        GetSystemInfo(&si);
        return si.dwPageSize;
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
        return sysconf(_SC_PAGESIZE);
#endif
    }

    void *AllocateOnPageAlignment(uint32_t pageSize, uint32_t blockSize) {
#if defined(_WIN32) || defined(_WIN64)
        return VirtualAlloc(nullptr, blockSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
        void* alignedBuffer = nullptr;
        assert(posix_memalign(&alignedBuffer, pageSize, blockSize) == 0);
        return alignedBuffer;
#endif
    }

    void FreeOnPageAlignment(void *block) {
#if defined(_WIN32) || defined(_WIN64)
        VirtualFree(block, 0, MEM_RELEASE);
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
        free(block);
#endif
    }
}
