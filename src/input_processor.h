#pragma once
#include <Arduino.h>
#include "io_buffer.h"
#include "logging.h"

namespace Cyber
{
    class InputProcessor
    {
        const float Inv12Bit = 1.0 / 4095.0;
        const int GateThresLow = 10;
        const int GateThresHigh = 245;
        const float CvRange = 8;
        const float ModRange = 5;
        
        int InputGate[4];
        bool Gate[4];

    public:
        // Adjustments
        int OffsetCv[4] = {0, 0, 0, 0};
        int OffsetMod[4] = {0, 0, 0, 0};
        float ScaleCv[4] = {1,1,1,1};
        float ScaleMod[4] = {1,1,1,1};
        int16_t GateSpeed = 64; // 1 = slowest, 255 = instant change, no filtering

        inline FpBuffer ConvertToFp(DataBuffer* buf)
        {
            FpBuffer output;
            for (int i = 0; i < buf->Size; i++)
            {
                output.Cv[0][i] = (float)((int)buf->Cv[0][i] - OffsetCv[0]) * ScaleCv[0] * Inv12Bit * CvRange;
                output.Cv[1][i] = (float)((int)buf->Cv[1][i] - OffsetCv[1]) * ScaleCv[1] * Inv12Bit * CvRange;
                output.Cv[2][i] = (float)((int)buf->Cv[2][i] - OffsetCv[2]) * ScaleCv[2] * Inv12Bit * CvRange;
                output.Cv[3][i] = (float)((int)buf->Cv[3][i] - OffsetCv[3]) * ScaleCv[3] * Inv12Bit * CvRange;

                output.Mod[0][i] = (float)((int)buf->Mod[0][i] - 2048 + OffsetMod[0]) * ScaleMod[0] * Inv12Bit * ModRange;
                output.Mod[1][i] = (float)((int)buf->Mod[1][i] - 2048 + OffsetMod[1]) * ScaleMod[1] * Inv12Bit * ModRange;
                output.Mod[2][i] = (float)((int)buf->Mod[2][i] - 2048 + OffsetMod[2]) * ScaleMod[2] * Inv12Bit * ModRange;
                output.Mod[3][i] = (float)((int)buf->Mod[3][i] - 2048 + OffsetMod[3]) * ScaleMod[3] * Inv12Bit * ModRange;
            }
            
            for (int idx = 0; idx < 4; idx++)
            {
                for (int i = 0; i < buf->Size; i++)
                {
                    InputGate[idx] += (-1+2*((int)buf->Gate[idx][i])) * GateSpeed;
                    if (InputGate[idx] > 255) InputGate[idx] = 255;
                    if (InputGate[idx] < 0) InputGate[idx] = 0;
                    if (InputGate[idx] < GateThresLow) Gate[idx] = false;
                    if (InputGate[idx] > GateThresHigh) Gate[idx] = true;
                    output.Gate[idx][i] = Gate[idx];
                }
            }
            return output;
        }
    };

    extern InputProcessor inProcessor;
}
