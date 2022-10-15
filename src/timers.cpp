#include "timers.h"

namespace Cyber
{
    PerfTimer perfAudio; 
    PerfTimer perfYield; 
    PerfTimer perfIo; 
    
    PerfTimer* GetPerfAudio() { return &perfAudio; }
    PerfTimer* GetPerfYield() { return &perfYield; }
    PerfTimer* GetPerfIo() { return &perfIo; }
}
