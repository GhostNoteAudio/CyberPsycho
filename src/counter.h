#pragma once
#include <Arduino.h>

namespace Cyber
{
    uint64_t GetCounter();
    void SpinWait(uint64_t cycles);
}