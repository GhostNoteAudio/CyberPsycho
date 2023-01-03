#pragma once
#include <stdint.h>
#include "wavetable.h"

namespace Modules
{
    class Fm4Osc
    {
        float ratio[4] = {1, 1, 1, 1};
        float offsetHz[4] = {0};
        float baseFreqHz = 200;

        uint32_t inc[4] = {0};
        uint32_t phasor[4] = {0};

        float output[4] = {0};

    public:
        float matrix[4][4] = {{0}}; // source->dest
        float volume[4] = {1, 0, 0, 0}; // output volume
        
        inline void SetOperator(int op, float ratio, float offset)
        {
            this->ratio[op] = ratio;
            this->offsetHz[op] = offset;

            UpdateIncrements();
        }

        inline void SetPitch(float freqHz)
        {
            baseFreqHz = freqHz;
            UpdateIncrements();
        }

        inline void UpdateIncrements()
        {
            for (int i = 0; i < 4; i++)
            {
                float hz = baseFreqHz * ratio[i] + offsetHz[i];
                inc[i] = Wavetable::GetPhaseIncrement(hz);
            }
        }

        inline float Process()
        {
            float newOutput[4];

            for (int i = 0; i < 4; i++)
            {
                phasor[i] += inc[i];
                float fm0 = output[0] * matrix[0][i];
                float fm1 = output[1] * matrix[1][i];
                float fm2 = output[2] * matrix[2][i];
                float fm3 = output[3] * matrix[3][i];
                auto phasemod = Wavetable::GetPhasor(fm0 + fm1 + fm2 + fm3);
                newOutput[i] = Wavetable::Sin(phasor[i] + phasemod);
            }

            output[0] = newOutput[0];
            output[1] = newOutput[1];
            output[2] = newOutput[2];
            output[3] = newOutput[3];
            
            return output[0] * volume[0]
                 + output[1] * volume[1]
                 + output[2] * volume[2]
                 + output[3] * volume[3];
        }
        
    };
}
