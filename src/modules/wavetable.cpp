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
    
    float Wavetable::SinFast(uint32_t phasor)
    {
        // p is the primary indexer into the 1024-point wavetable, it's a 10 bit value
        uint32_t p = phasor >> 22;
        // rem uses the next lower 8 bits to figure out the mix between p and p+1, in increments of 1/256
        uint32_t rem = (phasor >> 14) - (p << 8);
        float reminv = RemInv[rem]; // [0...255/256]

        float a = SinTable[p];
        float b = SinTable[(p + 1) & 0x3FF]; // module 10 bits
        return a * (1-reminv) + b * reminv;
    }
}
