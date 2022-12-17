#pragma once
#include "stdint.h"

namespace Modules
{
    // Linear Feedback Shift Register random generator
    // Fairly unusual noise source, produces slight roll-off in higher frequencies due to autocorrelation
    class Lfsr
    {
    public:
        uint32_t Value;

        inline Lfsr()
        {
            Value = 0xFFFFFFFF;
        }

        inline void Update()
        {
            uint8_t a = (Value & 0b000001) > 0;
            uint8_t b = (Value & 0b000100) > 0;
            uint8_t c = (Value & 0b001000) > 0;
            uint8_t d = (Value & 0b100000) > 0;
            uint8_t inval = a ^ b ^ c ^ d;
            Value = (Value >> 1) | (inval << 31);
        }

        inline uint8_t GetUint8()
        {
            uint8_t* p = (uint8_t*)&Value;
            return *p;
        }

        inline int8_t GetInt8()
        {
            int8_t* p = (int8_t*)&Value;
            return *p;
        }

        inline uint16_t GetUint16()
        {
            uint16_t* p = (uint16_t*)&Value;
            return *p;
        }

        inline int16_t GetInt16()
        {
            int16_t* p = (int16_t*)&Value;
            return *p;
        }

        inline float GetFloat()
        {
            const float scaler = 1.0 / 0xFFFFFFFF;
            return Value * scaler;
        }
    };
}
