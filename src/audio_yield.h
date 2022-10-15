#pragma once
#include <functional>
#include "io_buffer.h"
#include "counter.h"

extern std::function<void(DataBuffer* data)> HandleAudioCb;

void yieldAudio();

PerfTimer* GetPerfAudio();
PerfTimer* GetPerfYield();
