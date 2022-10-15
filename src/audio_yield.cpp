#include "audio_yield.h"
#include "audio_io.h"
#include "logging.h"
#include "timers.h"

namespace Cyber
{
    std::function<void(DataBuffer* data)> HandleAudioCb = 0;
    
    void YieldAudio()
    {
        GetPerfYield()->Stop();

        if (audio.Available())
        {
            GetPerfAudio()->Start();
            auto buf = audio.BeginAudioProcessing();
            if (HandleAudioCb == 0)
            {
                LogInfo("HandleAudioCb has not been set!");
                return;
            }

            HandleAudioCb(buf);
            audio.EndAudioProcessing();
            GetPerfAudio()->Stop();
        }

        GetPerfYield()->Start();
    }
}