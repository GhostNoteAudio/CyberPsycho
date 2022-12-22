#pragma once
#include <Arduino.h>
#include "io_buffer.h"

namespace Cyber
{
    namespace Scope
    {
        extern uint16_t data[128];
        extern uint8_t channel;
        extern uint8_t downsampling;
        extern float movingAverage;
        extern void ProcessScope(DataBuffer* data);
    }
}
