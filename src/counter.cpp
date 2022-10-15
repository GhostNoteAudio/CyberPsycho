#include "counter.h"
#include "arm_math.h"
#include "core_pins.h"
#include <Arduino.h>

namespace Cyber
{
    uint64_t GetCounter()
    {
        uint32_t currentVal = ARM_DWT_CYCCNT;
        return currentVal;
    }

    void SpinWait(uint64_t cycles)
    {
        auto start = GetCounter();
        while (GetCounter() - start < cycles)
        {
        }
    }
}
