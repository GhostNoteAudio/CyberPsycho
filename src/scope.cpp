#include "scope.h"

namespace Cyber
{
    namespace Scope
    {
        uint16_t data[128];
        uint8_t channel = 7;
        uint8_t downsampling = 2;
        uint16_t triggerFreq = 10000;

        uint32_t scopeWritePtr = 0;
        uint32_t scopeCounter = 0;

        void ProcessScope(DataBuffer* db)
        {
            int dsEffective = (1<<downsampling);
            for (int i = 0; i < db->Size; i++)
            {
                if (scopeCounter % dsEffective == 0)
                {
                    if (scopeWritePtr < 128)
                    {
                        if (channel == 0) data[scopeWritePtr] = db->Cv[0][i];
                        else if (channel == 1) data[scopeWritePtr] = db->Cv[1][i];
                        else if (channel == 2) data[scopeWritePtr] = db->Cv[2][i];
                        else if (channel == 3) data[scopeWritePtr] = db->Cv[3][i];
                        else if (channel == 4) data[scopeWritePtr] = db->Mod[0][i];
                        else if (channel == 5) data[scopeWritePtr] = db->Mod[1][i];
                        else if (channel == 6) data[scopeWritePtr] = db->Mod[2][i];
                        else if (channel == 7) data[scopeWritePtr] = db->Mod[3][i];
                    }
                    scopeWritePtr++;
                }

                if ((scopeCounter % triggerFreq == 0) && scopeWritePtr >= 128)
                    scopeWritePtr = 0;

                scopeCounter++;
            }
        }
    }
}