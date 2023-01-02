#pragma once
#define _USE_MATH_DEFINES
#include <math.h>

namespace Modules
{
    class BlitOsc
    {
        static const int WINDOW_SIZE = 4;
        static const int WINDOW_SAMPLES = 2 * WINDOW_SIZE + 1;
        static const int WINDOW_ARRAY_LEN = 8;
        static const int OFFSET_COUNT = 128;

        static float sincWindows[OFFSET_COUNT][WINDOW_SAMPLES];

        static inline float sinc(float x)
        {
            if (x == 0) return 1;
            float k = M_PI * x;
            return sinf(k) / k;
        }

        static inline void ApplyHamming(float* buffer, int size)
        {
            int M = size - 1;

            for (int n = 0; n < size; n++)
            {
                float val = 0.54 - 0.46 * cosf(2 * M_PI * n / (double)M) + 0.08 * cosf(4 * M_PI * n / (double)M);
                buffer[n] *= val;
            }
        }

        // Calculates a windowed sinc pulse
        // dataDest: where to write the result
        // offset: fractional delay of which to delay the sinc pulse by. Must be between 0 and 1
        // size: number of samples
        static inline void Sincwindow(float* dataDest, float offset, int size)
        {
            for (int i = -size; i <= size; i++)
            {
                float v = sinc(i + offset);
                dataDest[i + size] = v;
            }

            ApplyHamming(dataDest, 2 * size + 1);

            float sum = 0.0;
            for (int i = 0; i < 2 * size + 1; i++)
                sum += dataDest[i];

            float sumInv = 1 / sum;
            for (int i = 0; i < 2 * size + 1; i++)
            {
                dataDest[i] *= sumInv;
            }
        }

        static inline void CreateWindows()
        {
            // Here, we pre-calculate sinc pulses with fractional delays from 0 to 1 samples of delay. we calculate 128 of them
            // Example; 0th pulse is delayed by 0 samples, 64th pulse is delayed by 0.5 samples, 127th pulse is delay by (almost) 1 sample (127/128 to be exact).
            for (int i = 0; i < OFFSET_COUNT; i++)
            {
                Sincwindow(&sincWindows[i][0], i / (float)OFFSET_COUNT, 4);
            }
        }

        struct Impulse
        {
            float offset;
            float gain;

            inline Impulse()
            {
                offset = 0;
                gain = 0;
            }

            inline Impulse(float offset, float gain)
            {
                this->offset = offset;
                this->gain = gain;
            }
        };

        struct WindowInstance
        {
            int idx;
            float* window;
            float gain;

            inline WindowInstance()
            {
                idx = 0;
                window = 0;
                gain = 0;
            }

            inline WindowInstance(int idx, float* window, float gain)
            {
                this->idx = idx;
                this->window = window;
                this->gain = gain;
            }
        };

    public:
        enum class Waveform
        {
            Saw = 0,
            Pulse = 1,
        };

    private:
        Waveform waveform;
        float period;
        bool enableBlit;
        float idx;
        float output;
        int pulseIdx;
        WindowInstance windows[WINDOW_ARRAY_LEN];
        int nextWindowIdx;
        Impulse train[2];
        int trainSize;
        float baseOffset;
        float offset;
        float integratedOutput;
        float integratorAlpha;

    public:

        // Bandlimited Impulse Train oscillator. Supports saw and pulse mode
        // sawOrPulse: false for saw, true for pulse
        // period: cycle period of the oscillator, in samples.
        // enableBlit: set to false to generate a naive impulse train, rounding each impulse to the next sample
        inline BlitOsc(Waveform waveform, float period, bool enableBlit = true)
        {
            CreateWindows();
            this->waveform = waveform;

            this->period = period;
            this->enableBlit = enableBlit;
            nextWindowIdx = 0;
            output = 0;
            idx = -1;
            pulseIdx = 0;
            integratedOutput = -0.5;

            if (waveform == Waveform::Saw)
            {
                train[0].offset = 0;
                train[0].gain = 1;
                train[1].offset = 1;
                train[1].gain = 1;
                trainSize = 2;
                baseOffset = 1;
                integratorAlpha = 0.995;
            }
            else // pulse
            {
                train[0].offset = 0.00;
                train[0].gain = 1;
                train[1].offset = 0.5;
                train[1].gain = -1;
                trainSize = 2;
                baseOffset = 0;
                integratorAlpha = 1;
            }

            setPeriod(period);

            for (int i = 0; i < WINDOW_ARRAY_LEN; i++)
                windows[i].idx = WINDOW_SAMPLES; // index out of bounds
        }

        // Update the period.
        // period = samplerate/frequency.
        inline void setPeriod(float newPeriod)
        {
            float currentWaveformFraction = idx / period;

            period = newPeriod;
            offset = -baseOffset / period;

            // here we set the index so that it's at the same phase in the wave, according to the new period length, as it was before
            idx = period * currentWaveformFraction;
        }

        inline void setPwm(float ratio)
        {
            if (waveform == Waveform::Saw)
            {
                if (ratio == 0) ratio = 1;
                float gainBoost = ratio < 0.5 ? (1 + ratio * 2) : (1 + (1 - ratio) * 2);
                train[1].offset = ratio;
                train[1].gain = ratio * gainBoost;
                train[0].gain = ratio < 1 ? (1 - ratio) * gainBoost : 1;
                baseOffset = gainBoost;
                setPeriod(period);
            }
            else if (waveform == Waveform::Pulse)
            {
                if (ratio < 0.01) ratio = 0.01;
                if (ratio > 0.99) ratio = 0.99;
                train[1].offset = ratio;
            }
        }

    private:
        // Gets the next impulse, or a dummy impulse out of bounds if there are no further pulses until the waveform cycles around
        inline Impulse nextPulse()
        {
            if (pulseIdx >= trainSize)
                return Impulse(1, 0);

            return train[pulseIdx];
        }

        // Gets the correct sinc pulse to use given the fractional delay (offset) of ideal impulse location vs. current index
        inline float* getWindow(float offset)
        {
            int windowIdx = (int)(offset * OFFSET_COUNT);
            if (windowIdx < 0 || windowIdx >= OFFSET_COUNT)
            {
                //this can happen if we modify the period size, accept one slightly bad sample
                windowIdx = 127;
            }

            float* wnd = &sincWindows[windowIdx][0];
            return wnd;
        }

        // queues a new impulse window to be written to the output.
        // if enableBlit is turned off, this is not used, and we just write one sample impulse directly to the output
        inline void startWindow(WindowInstance newWindow)
        {
            windows[nextWindowIdx] = newWindow;
            nextWindowIdx = (nextWindowIdx + 1) % WINDOW_ARRAY_LEN;
        }

    public:
        inline float tick()
        {
            idx++;
            output = offset;

            if (idx >= period)
            {
                idx -= period;
                pulseIdx = 0;
            }

            // In pulse mode, if the pulse can get very narrow, less than 1 sample. Thus we may have more than one impulse per index step.
            while (true)
            {
                float pidx = nextPulse().offset * period;

                if (idx >= pidx)
                {
                    float windowOffset = idx - pidx;
                    auto wnd = getWindow(windowOffset);
                    float impulseGain = nextPulse().gain;

                    if (enableBlit)
                        startWindow(WindowInstance(0, wnd, impulseGain));
                    else
                        output += impulseGain;

                    pulseIdx++;
                }
                else
                    break;
            }

            // apply the sinc windows
            for (int i = 0; i < WINDOW_ARRAY_LEN; i++)
            {
                auto w = &windows[i];
                if (w->idx >= WINDOW_SAMPLES) continue;
                output += w->window[w->idx] * w->gain;
                w->idx++;
            }

            // integrate the output impulse train
            integratedOutput *= integratorAlpha;
            integratedOutput += output;
            return integratedOutput;
        }
    };

}
