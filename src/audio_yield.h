#pragma once
#include "counter.h"
#include "io_buffer.h"
#include <functional>

namespace Cyber
{
    extern std::function<void(DataBuffer* data)> HandleAudioCb;

    void yieldAudio();

    PerfTimer* GetPerfAudio();
    PerfTimer* GetPerfYield();
}
