#include "wavetable.h"
#include <math.h>
#include "logging.h"

namespace Modules
{
    float SinTable[1024];
    float RemInv[256];

    void Wavetable::Init()
    {
        for (int i = 0; i < 1024; i++)
        {
            SinTable[i] = sin(i / 1024.0 * 2 * M_PI);
        }

        for (int i = 0; i < 256; i++)
        {
            RemInv[i] = i / 256.0;
        }   
    }

    float Wavetable::Lookup(uint32_t phasor, float* table)
    {
        // p is the primary indexer into the 1024-point wavetable, it's a 10 bit value
        uint32_t p = phasor >> 22;
        // rem uses the next lower 8 bits to figure out the mix between p and p+1, in increments of 1/256
        uint32_t rem = (phasor >> 14) - (p << 8);
        float reminv = RemInv[rem]; // [0...255/256]

        float a = table[p];
        float b = table[(p + 1) & 0x3FF]; // module 10 bits
        return a * (1-reminv) + b * reminv;
    }
    
    float Wavetable::Sin(uint32_t phasor)
    {
        return Lookup(phasor, SinTable);
    }

    float Wavetable::Pulse(uint32_t phasor, uint32_t width)
    {
        return (phasor < width) ? 1.0f : -1.0f;
    }

    float Wavetable::Triangle(uint32_t phasor)
    {
        const float scaler = 1.0 / 0x3FFFFFFF;

        int32_t quart = 0x3FFFFFFF & phasor;
        int k = 0;
        if (phasor < 0x3FFFFFFF)
            k = quart;
        else if (phasor < 0x7FFFFFFF)
            k = 0x3FFFFFFF - quart;
        else if (phasor < 0xBFFFFFFF)
            k = -quart;
        else
            k = quart - 0x3FFFFFFF;
        return k * scaler;
    }

    float Wavetable::Saw(uint32_t phasor)
    {
        const float scaler = -1.0 / 0x80000000;
        int32_t k = (int32_t)phasor;
        return k * scaler;
    }
}
