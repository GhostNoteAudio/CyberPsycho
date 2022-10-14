#pragma once
#include <Arduino.h>

uint64_t GetCounter();
void SpinWait(uint64_t cycles);

// Very accurate sub-microsecond timer for short periods
class PerfTimer
{
    int64_t a, b;
    int aMillis, bMillis;
    int periodMaxTimestamp = 0;

    float period = 0;
    float periodMax = 0;
    float periodAvg = 0;
    
    bool maxExpired(int bMillis)
    {
        // hold for 3 seconds
        return (bMillis - periodMaxTimestamp) > 3000;
    }
public:
    inline void Start()
    {
        a = GetCounter();
        aMillis = millis();
    }

    inline void Stop()
    {
        b = GetCounter();
        bMillis = millis();
        double periodLocal = 0;

        // fall back to less precise timer for long periods > 10 sec
        if (bMillis - aMillis > 10000)
        {
            periodLocal = (bMillis - aMillis) * 1000;
        }
        else
        {
            if (b < a)
                b += UINT32_MAX;
            periodLocal = ((double)(b-a)) / F_CPU * 1000000.0;
        }
        
        if (periodLocal > periodMax)
        {
            periodMax = periodLocal;
            periodMaxTimestamp = bMillis;
        }

        if (maxExpired(bMillis))
        {
            periodMax = periodLocal;
            periodMaxTimestamp = bMillis;
        }

        period = periodLocal;
        periodAvg = periodAvg * 0.99 + periodLocal * 0.01;
    }

    inline double Period()
    {
        return period;
    }

    inline double PeriodMax()
    {
        return periodMax;
    }

    inline double PeriodAvg()
    {
        return periodAvg;
    }
};
