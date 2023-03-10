#include "timers.h"
#include "logging.h"
namespace Cyber
{
    PerfTimer perfAudio; 
    PerfTimer perfYield; 
    PerfTimer perfIo; 
    
    PerfTimer* GetPerfAudio() { return &perfAudio; }
    PerfTimer* GetPerfYield() { return &perfYield; }
    PerfTimer* GetPerfIo() { return &perfIo; }

    float GetCpuLoad()
    {
        float ioLoad = perfIo.PeriodAvg() / MaxTimeInterruptMicros;
        //float processYieldLoad = (perfAudio.PeriodAvg() + perfYield.PeriodDecay()) / MaxTimeAudioProcessingMicros;
        float processYieldLoad = perfAudio.PeriodAvg() / MaxTimeAudioProcessingMicros;
        float totalLoad = ioLoad + processYieldLoad;
        return totalLoad;
    }
}
