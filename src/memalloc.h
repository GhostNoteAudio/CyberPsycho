#pragma once
#include "Arduino.h"
#include "stdint.h"

namespace Cyber
{
    class MemAlloc;

    class MemoryBlock
    {
        friend class MemAlloc;
        uint8_t* Ptr = 0;
        int Size = 0;
    public:
        MemoryBlock(int sizeBytes);
        ~MemoryBlock();
        uint8_t* GetPtr() { return Ptr; }
        int GetSize() { return Size; }
    };

    class MemAlloc
    {
        const static int BLOCK_SIZE = 1024 * 350;
        friend class MemoryBlock;
        MemoryBlock* Blocks[64] = {0};
        uint8_t Memory[BLOCK_SIZE] = {0};
        uint8_t* FreePtr = 0;
        int FreeSize = 0;
    public:
        MemAlloc();

    private:
        void Allocate(MemoryBlock* block);
        void Deallocate(MemoryBlock* block);
    };

    extern MemAlloc memAlloc;
}
