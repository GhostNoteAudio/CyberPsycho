#include "audio_yield.h"
#include "audio_io.h"
#include "logging.h"

std::function<void(DataBuffer* data)> HandleAudioCb = 0;
PerfTimer perfAudio; // measures the length of time actually spent processing audio
PerfTimer perfYield; // measures the maximum length of time between calls to yield, blocked by other operations

void yieldAudio()
{
    perfYield.Stop();

    if (audio.Available())
    {
        perfAudio.Start();
        auto buf = audio.BeginAudioProcessing();
        if (HandleAudioCb == 0)
        {
            LogInfo("HandleAudioCb has not been set!");
            return;
        }
        
        HandleAudioCb(buf);
        audio.EndAudioProcessing();
        perfAudio.Stop();
    }

    perfYield.Start();
}

PerfTimer* GetPerfAudio()
{
    return &perfAudio;
}

PerfTimer* GetPerfYield()
{
    return &perfYield;
}