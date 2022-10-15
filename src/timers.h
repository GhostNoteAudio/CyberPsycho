#pragma once
#include "counter.h"
#include "constants.h"

namespace Cyber
{
    class PerfTimer;
    
    // measures the length of time actually spent processing audio
    PerfTimer* GetPerfAudio();
    // measures the maximum length of time between calls to yield, blocked by other operations
    PerfTimer* GetPerfYield();
    // Measures the length of time taken in the interrupt loop that sets up SPI comms with ADC/DAC
    PerfTimer* GetPerfIo();
    // Returns estimated CPU Load by taking the max yield time, the average audio time and the IO time and summing together
    float GetCpuLoad();

    const float MaxTimeInterruptMicros = 1000000.0 / SAMPLERATE;
    const float MaxTimeAudioProcessingMicros = 1000000.0 * BUFFER_SIZE / SAMPLERATE;

    // Very accurate sub-microsecond timer for short periods
    class PerfTimer
    {
        int64_t a, b;
        int aMillis, bMillis;
        int periodMaxTimestamp = 0;

        float period = 0;
        float periodMax = 0;
        float periodAvg = 0;
        float periodDecay = 0;
        
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

            if (periodLocal > periodDecay)
            {
                periodDecay = periodLocal;
            }

            if (maxExpired(bMillis))
            {
                periodMax = periodLocal;
                periodMaxTimestamp = bMillis;
            }

            period = periodLocal;
            periodAvg = periodAvg * 0.99 + periodLocal * 0.01;
            periodDecay *= 0.99999;
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

        inline double PeriodDecay()
        {
            return periodDecay;
        }
    };

    
}