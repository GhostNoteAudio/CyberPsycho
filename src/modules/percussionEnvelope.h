#pragma once
#include "logging.h"
#include "fast_curves.h"

namespace Modules
{
    class PercussionEnvelope
    {
    private:
        bool currentGate = false;
        float DecayMultiplier = 0.99;
        float SlewRateAttack = 1;
        float Output = 0;
        float DecayOutput = 0;
        float FloorScaler = 1;
        float ValueFloor = 0;
        
    public:
        float DecaySamples = 40000;
        float AttackRate = 10000;
        float ValueFloorDb = -60;

        inline void UpdateParams()
        {
            const float div20 = (1.0 / 20.0);
            ValueFloor = pow10f(ValueFloorDb * div20);

            // calculate the multiplier M, as so:
            // 1.0 * M^Samples = ExpMin  - for decay and release
            DecayMultiplier = 1.0f / pow10f((-ValueFloorDb/DecaySamples) * div20);
            SlewRateAttack = 1.0f / AttackRate;
            FloorScaler = 1.0f / (1.0f - ValueFloor);
        }

    public:
        inline float Process(bool gate)
        {
            bool trig = !this->currentGate && gate;

            if (trig)
                DecayOutput = 1;

            DecayOutput *= DecayMultiplier;
            this->currentGate = gate;

            if (DecayOutput > Output)
                Output += SlewRateAttack;
            else
                Output = DecayOutput;

            if (Output < ValueFloor)
                Output = ValueFloor;

            return (Output - ValueFloor) * FloorScaler;
        }
    };
}
