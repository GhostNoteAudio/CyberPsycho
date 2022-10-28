#pragma once

#include "Arduino.h"
#include <cmath>
#include <algorithm>

namespace Cyber
{
    namespace Utils
    {
        inline uint16_t To12Bit(float value)
        {
            return 2048 + value * 2047;
        }

        inline void To12Bit(uint16_t* dest, const float* source, int size)
        {
            int i = 0;
            while(i < size)
            {
                dest[i] = 2048 + source[i] * 2047;
                i++;
            }
        }

        template<typename T>
        inline T Clip1(T value)
        {
            return value > 1 ? 1 : (value < -1 ? -1 : value);
        }

        template<typename T>
        inline T ClipF(T value, T min, T max)
        {
            return value > max ? max : (value < min ? min : value);
        }

        template<typename T>
        inline void Clip1(T* value, int len)
        {
            for (int i = 0; i < len; i++)
            {
                T v = value[i];
                value[i] = v > 1 ? 1 : (v < -1 ? -1 : v);
            }
        }

        inline int ClipI(int value, int min, int max)
        {
            return value > max ? max : (value < min ? min : value);
        }

        template<typename T>
        inline void ZeroBuffer(T* buffer, int len)
        {
            for (int i = 0; i < len; i++)
                buffer[i] = 0;
        }

        // inline void ZeroBuffer(int16_t* buffer, int len)
        // {
        //     for (int i = 0; i < len; i++)
        //         buffer[i] = 0;
        // }

        template<typename T>
        inline void Copy(T* dest, T* source, int len)
        {

            memcpy(dest, source, len * sizeof(T));
        }

        template<typename T>
        inline void Gain(T* buffer, T gain, int len)
        {
            for (int i = 0; i < len; i++)
            {
                buffer[i] *= gain;
            }
        }

        template<typename T>
        inline void Mix(T* target, T* source, T gain, int len)
        {
            for (int i = 0; i < len; i++)
                target[i] += source[i] * gain;
        }

        inline float MaxAbsF(float* data, int len)
        {
            float max = -99999999;
            for (int i = 0; i < len; i++)
            {
                float val = fabsf(data[i]);
                max = val > max ? val : max;
            }
            return max;
        }

        template<typename T>
        inline T Max(T* data, int len)
        {
            T max = std::numeric_limits<T>::min();
            for (int i = 0; i < len; i++)
            {
                T val = data[i];
                max = val > max ? val : max;
            }
            return max;
        }

        template<typename T>
        inline T Min(T* data, int len)
        {
            T min = std::numeric_limits<T>::max();
            for (int i = 0; i < len; i++)
            {
                T val = data[i];
                min = val < min ? val : min;
            }
            return min;
        }

        inline float Max(float* data, int len)
        {
            float max = -std::numeric_limits<float>::max();
            for (int i = 0; i < len; i++)
            {
                float val = data[i];
                max = val > max ? val : max;
            }
            return max;
        }

        inline float Min(float* data, int len)
        {
            float min = std::numeric_limits<float>::max();
            for (int i = 0; i < len; i++)
            {
                float val = data[i];
                min = val < min ? val : min;
            }
            return min;
        }

        inline double Max(double* data, int len)
        {
            double max = -std::numeric_limits<double>::max();
            for (int i = 0; i < len; i++)
            {
                double val = data[i];
                max = val > max ? val : max;
            }
            return max;
        }

        inline double Min(double* data, int len)
        {
            double min = std::numeric_limits<double>::max();
            for (int i = 0; i < len; i++)
            {
                double val = data[i];
                min = val < min ? val : min;
            }
            return min;
        }

        template<typename T>
        inline T Mean(T* data, int len)
        {
            double sum = 0;
            for (int i = 0; i < len; i++)
            {
                sum += data[i];
            }
            return (T)(sum / len);
        }

        inline float DB2Gainf(float input)
        {
            //return std::pow(10.0f, input / 20.0f);
            return std::pow(10.0f, input * 0.05f);
        }

        template<typename T>
        inline double DB2Gain(T input)
        {
            return std::pow(10, input / 20.0);
        }

        template<typename T>
        inline double Gain2DB(T input)
        {
            //if (input < 0.0000001)
            //    return -100000;

            return 20.0f * std::log10(input);
        }

        inline double Response4Oct(double input)
        {
            return std::min((std::pow(16, input) - 1.0) * 0.066666666667, 1.0);
        }

        inline double Response2Dec(double input)
        {
            return std::min((std::pow(100, input) - 1.0) * 0.01010101010101, 1.0);
        }

        // Truncates the end of an IR using a cosine window
        inline void TruncateCos(float* data, int dataSize, float fraction)
        {
            int sample_count = (int)(fraction * dataSize);
            int offset = dataSize - sample_count;
            for (int i = 0; i < dataSize; i++)
            {
                data[offset + i] *= cosf(i / (float)sample_count * M_PI*0.5);
            }
        }

        inline void ApplyHamming(float* buffer, int M)
        {
            for (int n = 0; n < M; n++)
            {
                float val = 0.42 - 0.5 * cosf(2*M_PI * n / (double)M) + 0.08 * cosf(4 * M_PI * n / (double)M);
                buffer[n] *= val;
            }
        }

        inline float Sinc(float x)
        {
            return x == 0 ? 1 : sinf(M_PI*x) / (M_PI*x);
        }

        // Note: choose N as an odd number
        inline void MakeSincFilter(float* buffer, int N, float fmin, float fmax, int Fs)
        {
            fmin = fmin / (Fs*0.5);
            fmax = fmax / (Fs*0.5);

            for (int i = 0; i < N; i++)
            {
                int x = i - N/2;
                float val = fmax * Sinc(x * fmax) - fmin*Sinc(x * fmin);
                buffer[i] = val;
            }

            ApplyHamming(buffer, N);
        }
    }
}
