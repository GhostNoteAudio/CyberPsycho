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
        float processYieldLoad = (perfAudio.PeriodAvg() + perfYield.PeriodDecay()) / MaxTimeAudioProcessingMicros;
        float totalLoad = ioLoad + processYieldLoad;

        LogInfof("ioLoad: %.3f :: processYieldLoad: %.3f", ioLoad, processYieldLoad)
        return totalLoad;
    }
}
