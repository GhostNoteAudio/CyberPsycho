#include <Arduino.h>
#include "counter.h"

uint32_t oldLow = ARM_DWT_CYCCNT;
uint32_t curHigh = 0;

uint64_t GetCounter()
{
    uint32_t curLow = ARM_DWT_CYCCNT;
    if (curLow < oldLow) // we had a roll over
    {
        curHigh++;
    }
    oldLow = curLow;
    uint64_t curVal = ((uint64_t)curHigh << 32) | curLow;

    return curVal;
}

void SpinWait(uint64_t cycles)
{
    auto start = GetCounter();
    while(GetCounter() - start < cycles)
    {

    }
}
