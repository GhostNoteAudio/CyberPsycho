#include "memalloc.h"
#include "logging.h"

namespace Cyber
{
    DMAMEM MemAlloc memAlloc;

    MemoryBlock::MemoryBlock(int sizeBytes)
    {
        LogInfof("Creating memory block of size %d", sizeBytes);
        Size = sizeBytes;
        memAlloc.Allocate(this);
    }

    MemoryBlock::~MemoryBlock()
    {
        LogInfo("Deallocating memory block");
        memAlloc.Deallocate(this);
        Size = 0;
        Ptr = 0;
    }

    MemAlloc::MemAlloc()
    {
        FreePtr = Memory;
        FreeSize = BLOCK_SIZE;
    }

    void MemAlloc::Allocate(MemoryBlock* block)
    {
        auto findEmptySlot = [this]()
        {
            for (int i = 0; i < 64; i++)
            {
                if (Blocks[i] == 0)
                    return i;
            }
            return -1;
        };

        int slot = findEmptySlot();

        if (block->Size > FreeSize || slot == -1)
        {
            block->Size = 0;
            block->Ptr = 0;
            return;
        }

        Blocks[slot] = block;
        block->Ptr = FreePtr;
        FreePtr = FreePtr + block->Size;
        FreeSize -= block->Size;
        LogInfof("Allocated %d byte, freesize: %d", block->Size, FreeSize);
    }

    void MemAlloc::Deallocate(MemoryBlock* block)
    {
        LogInfof("Deallocating block at %d, size %d", block->Ptr, block->Size);
        auto ptr = block->Ptr;
        auto size = block->Size;
        block->Ptr = 0;
        block->Size = 0;

        if (ptr == 0 || size == 0)
        {
            LogInfo("Deallocated empty block");
            return;
        }

        MemoryBlock* maxBlock = 0;

        // update pointers in higher blocks, shift them down to match new memory layout
        for (int i = 0; i < 64; i++)
        {
            if (Blocks[i] == block)
                Blocks[i] = 0;

            if (Blocks[i] == 0)
                continue;

            if (Blocks[i]->Ptr > ptr)
            {
                Blocks[i]->Ptr -= size;
                if (maxBlock == 0 || Blocks[i]->Ptr > maxBlock->Ptr)
                {
                    maxBlock = Blocks[i];
                    LogInfof("Max Block at %d", maxBlock->Ptr);
                }
            }
        }

        if (maxBlock != 0)
        {
            uint8_t* maxPtr = maxBlock->Ptr + maxBlock->Size - 1;

            LogInfof("We must shift memory between %d and %d", ptr, maxPtr);
            // shift memory above block down
            while(true)
            {
                uint8_t* source = ptr + size;
                if (source > maxPtr)
                    break;

                *ptr = *source;
                ptr++;
            }
        }
        else
        {
            LogInfo("Block was the last block, nothing to shift");
        }

        FreePtr -= size;
        FreeSize += size;
        LogInfof("Deallocated %d bytes, freesize: %d", size, FreeSize);
    }
}
