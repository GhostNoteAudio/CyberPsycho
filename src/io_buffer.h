#pragma once
#include "constants.h"

namespace Cyber
{
    template<int T>
    class IOBuffer
    {
    public:
        const int Size = T;
        uint8_t Gate[4][T] = {{0}};
        uint16_t Cv[4][T] = {{0}};
        uint16_t Mod[4][T] = {{0}};
        uint16_t Out[4][T] = {{0}};
    };

    template<int T>
    class FloatBuffer
    {
    public:
        const int Size = T;
        bool Gate[4][T] = {{0}};
        float Cv[4][T] = {{0}};
        float Mod[4][T] = {{0}};
        float Out[4][T] = {{0}};
    };

    typedef IOBuffer<BUFFER_SIZE> DataBuffer;
    typedef FloatBuffer<BUFFER_SIZE> FpBuffer;
}