// knob = 0.9
// data = np.ones(10000)

// if knob > 0.5:
//     decay_len = len(data) * (1-knob)*2
//     decay_start = len(data) - decay_len
//     decay_end = len(data) - 1
// else:
//     decay_len = len(data) * (knob*2)
//     decay_start = 0
//     decay_end = decay_len - 1
    
// gain = 1
// gain_loss_per_sample = 1.0/decay_len * 60
// multiplier = np.power(10.0, -gain_loss_per_sample / 20.0)

// for i in range(len(data)):
//     if i >= decay_start and i <= decay_end:
//         gain *= multiplier
//     data[i] *= gain

#pragma once
#include "logging.h"
#include "fast_curves.h"

namespace Modules
{
    class SampleEnvelope
    {
    private:
        bool currentGate = false;
        float DecayMultiplier = 0.99;
        float Output = 0;
        float DecayStart = 0;
        float DecayEnd = 100;
        int Idx = 0;

    public:
        int SampleLength = 100;
        float Value = 0.1;

        inline void Update()
        {
            float decayLen;

            if (Value > 0.5)
            {
                decayLen = SampleLength * (1.0-Value)*2.0;
                DecayStart = SampleLength - decayLen;
                DecayEnd = SampleLength - 1;
            }
            else
            {
                decayLen = SampleLength * (Value*2);
                DecayStart = 0;
                DecayEnd = decayLen - 1;
            }

            float gainLossPerSample = 1.0/decayLen * 60;
            DecayMultiplier = pow10f(-gainLossPerSample/ 20.0);
        }

    public:
        inline float Process(bool gate)
        {
            bool trig = !this->currentGate && gate;
            this->currentGate = gate;
            if (trig)
            {
                Output = 1;
                Idx = 0;
            }

            if (Idx >= DecayStart && Idx <= DecayEnd)
                Output *= DecayMultiplier;
            
            Idx++;
            return Output;
        }
    };
}
