
#ifndef GREYPIXELENGINE_CPUWORK_PAGEALLOCATION_H
#define GREYPIXELENGINE_CPUWORK_PAGEALLOCATION_H

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__unix__) || defined(__unix) || defined(__APPLE__) && defined(__MACH__)
#include <unistd.h>
#include <cstdlib>
#else
"ERROR: Unknown operating system"
#endif

#include <cstdint>
#include <cassert>

namespace GP {

    uint32_t NextPowerOfTwo(uint32_t n);


    uint64_t GetPageByteSize();
    void* AllocateOnPageAlignment(uint32_t pageSize, uint32_t blockSize);

    void FreeOnPageAlignment(void* block);


}

#endif //GREYPIXELENGINE_CPUWORK_PAGEALLOCATION_H
