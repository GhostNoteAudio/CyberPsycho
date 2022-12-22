#include "scope.h"

namespace Cyber
{
    namespace Scope
    {
        uint16_t data[128];
        uint8_t channel = 7;
        uint8_t downsampling = 2;

        uint32_t scopeWriteCounter = 0;
        uint32_t scopeCounter = 0;

        float movingAverage = 0;
        bool aboveAvg = false;

        void ProcessScope(DataBuffer* db)
        {
            int dsEffective = (1<<downsampling);

            uint16_t* inputData = 0;
            if (channel == 0) inputData = db->Cv[0];
            else if (channel == 1) inputData = db->Cv[1];
            else if (channel == 2) inputData = db->Cv[2];
            else if (channel == 3) inputData = db->Cv[3];
            else if (channel == 4) inputData = db->Mod[0];
            else if (channel == 5) inputData = db->Mod[1];
            else if (channel == 6) inputData = db->Mod[2];
            else if (channel == 7) inputData = db->Mod[3];

            for (int i = 0; i < db->Size; i++)
            {
                movingAverage = movingAverage * 0.999 + inputData[i] * 0.001;

                if (scopeCounter % dsEffective == 0)
                {
                    if (scopeWriteCounter < 128)
                    {
                        data[scopeWriteCounter] = inputData[i];
                    }
                    scopeWriteCounter++;
                }

                bool newAboveAvg = inputData[i] > movingAverage;
                // trigger condition:
                // * display has been filled
                // * crossing the moving average
                if (scopeWriteCounter >= 128 && !aboveAvg && newAboveAvg)
                    scopeWriteCounter = 0;

                scopeCounter++;
                aboveAvg = newAboveAvg;
            }
        }
    }
}