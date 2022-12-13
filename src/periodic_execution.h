#pragma once
#include <Arduino.h>

namespace Cyber
{
    class PeriodicExecution
    {
        int period = 1000;
        int lastTs = 0;
    public:
        bool active = true;

        PeriodicExecution(int millis)
        {
            period = millis;
        }

        void Set(int millis)
        {
            period = millis;
        }

        void Clear()
        {
            lastTs = millis();
        }

        bool Go()
        {
            int m = millis();
            if (m - lastTs >= period)
            {
                lastTs = m;
                return active;
            }
            return false;
        }
    };
}