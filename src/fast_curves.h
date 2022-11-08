#pragma once
#include "utils.h"

namespace Cyber
{
    namespace FastCurves
    {
        extern float Resp2Dec[256];

        inline void Init()
        {
            for (int i = 0; i < 256; i++)
            {
                Resp2Dec[i] = Utils::Resp2dec(i / 255.0);
            }
        }

        inline float Read(float* table, float pos)
        {
            if (pos >= 1)
                pos = 0.99999;

            float pos2 = pos * 255;

            int idxA = pos2;
            int idxB = idxA + 1;
            int rem = pos2 - idxA;
            return table[idxA] * (1-rem) + table[idxB] * rem;
        }
    }
}

