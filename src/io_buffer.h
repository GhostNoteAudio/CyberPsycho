#pragma once
#include <Arduino.h>
#include "constants.h"
#include "stdint.h"

namespace Cyber
{
    template<int T>
    struct DataBufferT
    {
        const int Size = T;
        uint8_t Gate[4][T] = {{0}};
        uint16_t Cv[4][T] = {{0}};
        uint16_t Mod[4][T] = {{0}};
        uint16_t Out[4][T] = {{0}};
    };

    template<int T>
    struct FpBufferT
    {
        const int Size = T;
        bool Gate[4][T] = {{0}};
        float GateFloat[4][T] = {{0}};
        float Cv[4][T] = {{0}};
        float Mod[4][T] = {{0}};
        float Out[4][T] = {{0}};

        inline void CopyTo(FpBufferT<T>* dest)
        {
            memcpy(dest->Gate, Gate, sizeof(Gate));
            memcpy(dest->GateFloat, GateFloat, sizeof(GateFloat));
            memcpy(dest->Cv, Cv, sizeof(Cv));
            memcpy(dest->Mod, Mod, sizeof(Mod));
            memcpy(dest->Out, Out, sizeof(Out));
        }
    };

    typedef DataBufferT<BUFFER_SIZE> DataBuffer;
    typedef FpBufferT<BUFFER_SIZE> FpBuffer;
}