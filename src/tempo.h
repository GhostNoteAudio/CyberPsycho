#pragma once
#include "constants.h"

namespace Cyber
{
    enum class TempoMode
    {
        Internal = 0,
        External = 1,
        Midi = 2
    };

    class TempoState
    {
        TempoMode tempoMode;
        float fixedBpmValue;

        float bpm; // calculated value, or = fixedBpmValue if Internal mode
        float bpmInv;
        
        int trigDivision = 1;

        int trigMinus4 = 0;
        int trigMinus3 = 0;
        int trigMinus2 = 0;
        int trigMinus1 = 0;
        int trigMinus0 = 0;

    public:
        inline float GetBpm() { return bpm; }
        inline void SetBpm(float newBpm) { fixedBpmValue = newBpm; UpdateBpm(); }

        inline TempoMode GetTempoMode() { return tempoMode; }
        inline void SetTempoMode(TempoMode mode) { tempoMode = mode; }

        inline void UpdateBpm()
        {
            if (tempoMode == TempoMode::Internal)
            {
                bpm = fixedBpmValue;
            }
            else if (tempoMode == TempoMode::External)
            {
                float triggerPeriod = (trigMinus0 - trigMinus4) / 4.0f;
                
                if (triggerPeriod < 10)
                    bpm = 300;
                else if (triggerPeriod > SAMPLERATE * 6)
                    bpm = 10;

                float bpm = SAMPLERATE /  (triggerPeriod * trigDivision) * 60;
            }
            else if (tempoMode == TempoMode::Midi)
            {
                // todo: handle midi sync
            }

            bpmInv = 1.0 / bpm;
        }

        inline void SetTrigDivision(int div)
        {
            trigDivision = div;
            UpdateBpm();
        }

        inline void SetTrig(int sampleOffset)
        {
            trigMinus4 = trigMinus3;
            trigMinus3 = trigMinus2;
            trigMinus2 = trigMinus1;
            trigMinus1 = trigMinus0;
            trigMinus0 = sampleOffset;
            UpdateBpm();
        }

        inline float GetSamplesPerNBote(int numerator, int denominator, bool triplet, bool dotted)
        {
            // quarter notes are the standard
            float samplesperq = 60 * SAMPLERATE * bpmInv;
            float samples = samplesperq * numerator;

            if (denominator == 1)
                samples *= 4;
            else if (denominator == 2)
                samples *= 2;
            //else if (denominator == 4)
            //    samples *= 1;
            else if (denominator == 8)
                samples *= 0.5;
            else if (denominator == 16)
                samples *= 0.25;
            else if (denominator == 32)
                samples *= 0.125;
            else if (denominator == 64)
                samples *= 0.0625;
            else if (denominator == 128)
                samples *= 0.03125;
            else // denominator must be power of 2, [1-128]
                return 1;

            if (triplet)
                return samples * 0.666667;
            if (dotted)
                return samples * 1.5;
            // todo: implement 
        }
    };

    extern TempoState tempoState;
}
