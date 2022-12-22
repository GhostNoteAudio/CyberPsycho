#pragma once
#include <stdint.h>
#include "generator.h"
#include "io_buffer.h"
#include "generators/bypass.h"
#include "generatorRegistry.h"
#include "menu.h"
#include "mod_matrix.h"
#include "input_processor.h"
#include "menus.h"

namespace Cyber
{
    class Voice
    {        
    public:        
        Generator* Gen = nullptr;
        float GainOut = 1.0f;
        int PitchOffset = 0;
        ModMatrix matrix;

        inline void Init()
        {
        }

        inline void SetGenerator(int genId, bool force = false)
        {
            if (genId == -1)
            {
                LogInfo("Invalid genId -1 passed in, unable to create generator");
                return;
            }

            if (Gen == nullptr || Gen->GenIndex != genId || force)
            {
                generatorRegistry.DeleteGenInstance(Gen);
                Gen = generatorRegistry.CreateGenInstance(genId);
                matrix.GetMenu()->Steps[1] = Gen->GetModSlots();
            }
        }

        inline void OverrideIfPreview(FpBuffer* data)
        {
            for (int btn = 0; btn < 4; btn++)
            {
                if (modalState.GateTrigger(btn))
                {
                    for (int i = 0; i < data->Size; i++)
                    {
                        data->Gate[btn][i] = true;
                        data->GateFloat[btn][i] = 1;
                    }
                }
            }
        }
        
        inline void Process(DataBuffer* data)
        {
            auto fpData = inProcessor.ConvertToFp(data);
            OverrideIfPreview(fpData);
            matrix.fpData = fpData;

            GeneratorArgs args;
            args.Bpm = 120;
            args.Data = fpData;
            args.PitchOffset = PitchOffset;
            args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(slot); };
            args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(slot); };
            Gen->Process(args);

            Utils::Gain(fpData->Out[0], GainOut, fpData->Size);
            Utils::Gain(fpData->Out[1], GainOut, fpData->Size);
            Utils::Gain(fpData->Out[2], GainOut, fpData->Size);
            Utils::Gain(fpData->Out[3], GainOut, fpData->Size);

            Utils::To12Bit(data->Out[0], fpData->Out[0], data->Size);
            Utils::To12Bit(data->Out[1], fpData->Out[1], data->Size);
            Utils::To12Bit(data->Out[2], fpData->Out[2], data->Size);
            Utils::To12Bit(data->Out[3], fpData->Out[3], data->Size);
        }
    };

    extern Voice voice;
}
