#pragma once
#include "constants.h"
#include "stdint.h"

namespace Modules
{
    const int PhaseScaler = 0xFFFFFFFF / Cyber::SAMPLERATE;

    class Wavetable
    {
    public:
        static void Init();
        static float SinFast(uint32_t phasor);
        inline static int GetPhaseIncrement(float hz)
        {
            return hz * PhaseScaler;
        }
    };
}
