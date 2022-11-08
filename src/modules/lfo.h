#pragma once
#include "logging.h"
#include "fast_curves.h"

namespace Modules
{
    class Lfo
    {
    public:
        enum class Shape
        {
            Sin = 0,
            Pulse,
            Triangle,
            Saw,
            Ramp,
            RandomStep,
            RandomSlew
        };

    private:
        bool currentGate;
        float samplerate;

    public:
        inline void SetSamplerate(float samplerate)
        {
            this->samplerate = samplerate;
            UpdateParams();
        }

        inline void UpdateParams()
        {
            
        }

        inline float Process(bool gate)
        {
            bool trig = !this->currentGate && gate;

            if (trig)
            {
                
            }

            this->currentGate = gate;
            return 0.0f;
        }
    };
}
