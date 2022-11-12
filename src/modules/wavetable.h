#pragma once
#include "constants.h"
#include "stdint.h"

namespace Modules
{
    const float PhaseScaler = ((double)0xFFFFFFFF) / Cyber::SAMPLERATE;

    class Wavetable
    {
    public:
        static void Init();
        static float Lookup(uint32_t phasor, float* table);
        static float Sin(uint32_t phasor);
        static float Pulse(uint32_t phasor, uint32_t width);
        static float Triangle(uint32_t phasor);
        static float Saw(uint32_t phasor);

        inline static int GetPhaseIncrement(float hz)
        {
            return hz * PhaseScaler;
        }

        inline static uint32_t GetPhasor(float pos)
        {
            return pos * 0xFFFFFFFF;
        }
    };
}
