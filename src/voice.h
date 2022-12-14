#pragma once
#include <stdint.h>
#include "generator.h"
#include "io_buffer.h"
#include "generators/bypass.h"
#include "generatorRegistry.h"
#include "menu.h"
#include "mod_matrix.h"
#include "input_processor.h"

namespace Cyber
{
    class Voice
    {        
    public:        
        Generator* Gen = nullptr;
        float GainOut[4] = {1.0f, 1.0f, 1.0f, 1.0f};
        int PitchOffset[4] = {0};
        ModMatrix matrix;

        inline void Init()
        {
            Gen = generatorRegistry.CreateInstanceById("Bypass");
        }

        inline void SetGenerator(int genId)
        {
            if (Gen->GenIndex != genId)
            {
                generatorRegistry.DeleteInstance(Gen);
                Gen = generatorRegistry.CreateInstance(genId);
            }
        }
        
        inline void Process(DataBuffer* data)
        {
            auto fpData = inProcessor.ConvertToFp(data);

            GeneratorArgs args;
            args.Bpm = 120;
            args.Data = fpData;
            args.PitchOffset[0] = PitchOffset[0];
            args.PitchOffset[1] = PitchOffset[1];
            args.PitchOffset[2] = PitchOffset[2];
            args.PitchOffset[3] = PitchOffset[3];
            args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(slot); };
            args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(slot); };
            Gen->Process(args);

            Utils::Gain(fpData->Out[0], GainOut[0], fpData->Size);
            Utils::Gain(fpData->Out[1], GainOut[1], fpData->Size);
            Utils::Gain(fpData->Out[2], GainOut[2], fpData->Size);
            Utils::Gain(fpData->Out[3], GainOut[3], fpData->Size);

            Utils::To12Bit(data->Out[0], fpData->Out[0], data->Size);
            Utils::To12Bit(data->Out[1], fpData->Out[1], data->Size);
            Utils::To12Bit(data->Out[2], fpData->Out[2], data->Size);
            Utils::To12Bit(data->Out[3], fpData->Out[3], data->Size);
        }

        /*inline bool IsGenMenu(Menu* menu)
        {
            bool b0 = menu ==  Gen->GetMenu(0);
            bool b1 = menu ==  Gen->GetMenu(1);
            bool b2 = menu ==  Gen->GetMenu(2);
            bool b3 = menu ==  Gen->GetMenu(3);
            return b0 || b1 || b2 || b3;
        }*/
    };

    extern Voice voice;
}
