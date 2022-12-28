#pragma once
#include "constants.h"
#include "stdint.h"
#include "logging.h"

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
        TempoMode tempoMode = TempoMode::External;
        float fixedBpmValue;

        float bpm = 0; // calculated value, or = fixedBpmValue if Internal mode
        float bpmInv;
        
        int trigDivision = 1;

        bool clkVal = false;
        int clkTicks = 0;

        int microsLast = 0;

        int trigMinus3 = 0;
        int trigMinus2 = 0;
        int trigMinus1 = 0;
        int trigMinus0 = 0;

    public:
        inline float GetBpm() { return bpm; }
        inline void SetInternalBpm(float newBpm) { fixedBpmValue = newBpm; UpdateBpm(); }
        inline float GetInternalBpm() { return fixedBpmValue; }

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
                float avgTriggerPeriodSamples = (trigMinus0 + trigMinus1 + trigMinus2 + trigMinus3) * 0.25f;
                bpm = SAMPLERATE / (avgTriggerPeriodSamples * trigDivision) * 60;
                if (isinff(bpm)) bpm = 120;
                if (bpm < 10) bpm = 10;
            }
            else if (tempoMode == TempoMode::Midi)
            {
                float avgTriggerPeriodSec = (trigMinus0 + trigMinus1 + trigMinus2 + trigMinus3) * 0.25f * 0.000001f;
                float newBpm = 60 / (avgTriggerPeriodSec * trigDivision);
                if (isinff(newBpm)) newBpm = 120;
                bpm = bpm * 0.95f + newBpm * 0.05f;
                if (bpm < 10) bpm = 10;
            }

            bpmInv = 1.0 / bpm;
        }

        inline void SetTrigDivision(int div)
        {
            trigDivision = div;
            UpdateBpm();
        }

        inline void TickClk(bool value)
        {
            if (tempoMode != TempoMode::External)
                return;

            if (value && !clkVal)
            {
                UpdateTrigger(clkTicks);
                clkTicks = 0;
            }

            clkVal = value;
            clkTicks++;
        }

        inline void TickMidi()
        {
            if (tempoMode != TempoMode::Midi)
                return;

            int ts = micros();
            int micros = ts - microsLast;
            if (micros < 0)
            {
                // overflow condition, skip
            }
            else if (micros < 3000)
            {
                // weird double send, sometimes happens when starting clock in some DAWs
            }
            else if (micros > 125000)
            {
                // long pause, over 125 ms (20BPM) between sync pulses, assume no signal
            }
            else
            {
                LogInfof("Updating with micros %d", micros);
                UpdateTrigger(micros);
            }
            microsLast = ts;
        }

        inline void UpdateTrigger(int sampleDuration)
        {
            trigMinus3 = trigMinus2;
            trigMinus2 = trigMinus1;
            trigMinus1 = trigMinus0;
            trigMinus0 = sampleDuration;
            UpdateBpm();
        }

        inline float GetSamplesPerNote(int numerator, int denominator, bool triplet, bool dotted)
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
