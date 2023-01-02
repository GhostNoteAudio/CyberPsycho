#pragma once
#include <stdint.h>
#include <math.h>
#include "constants.h"
#include "wavetable.h"

namespace Modules
{
    class AdditiveOsc
    {
        static const int HARMONIC_COUNT = 32;

        float harmonics[HARMONIC_COUNT] = {0};
        float gains[HARMONIC_COUNT] = {0};
        float frequency = 0;
        float maxHarmonic = 256;
        uint32_t inc = 0;
        uint32_t phasor = 0;

    public:
        inline AdditiveOsc()
        { }

        inline void SetFrequency(float frequency)
        {
            this->frequency = frequency;
            maxHarmonic = Cyber::SAMPLERATE / (2*frequency);
            inc = Wavetable::GetPhaseIncrement(frequency);
        }

        inline void SetSaw()
        {
            for (int i = 0; i < HARMONIC_COUNT; i++)
            {
                SetTone(i, i+1, 1.0/(i+1));
            }
        }

        inline void SetTriangle()
        {
            int sign = 1;
            const float piSquared = M_PI * M_PI;

            for (int i = 0; i < HARMONIC_COUNT; i++)
            {
                int n = (2*i + 1);
                float ff = (8.0f / (piSquared * (n*n))) * sign;
                sign *= -1;
                SetTone(i, n, ff);
            }
        }

        inline void SetTone(uint8_t idx, float harmonic, float gain)
        {
            if (idx >= HARMONIC_COUNT) return;

            harmonics[idx] = harmonic;
            gains[idx] = gain;
        }

        inline float Process()
        {
            phasor += inc;
            float sum = 0.0;
            for (int i = 0; i < HARMONIC_COUNT; i++)
            {
                float h = harmonics[i];
                if (h > maxHarmonic) continue;

                float g = gains[i];
                sum += Wavetable::Sin(phasor * h) * g;
            }
            
            return sum;
        }
    };
}
