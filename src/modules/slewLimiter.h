#pragma once
#include "math.h"

namespace Modules
{
    class SlewLimiter
    {
        float state = 0;
    public:
        float Rate = 10000;

        inline void Set(float newState)
        {
            state = newState;
        }

        inline float Update(float value)
        {
            float delta = value - state;
            if (delta > 0 && delta > Rate)
                delta = Rate;
            else if (delta < 0 && delta < -Rate)
                delta = -Rate;
            
            state += delta;
            return state;
        }
    };
}
