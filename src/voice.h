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
#include "modal_state.h"
#include "midi.h"

namespace Cyber
{
    class Voice
    {        
    public:        
        Generator* Gen = nullptr;
        float GainOut = 1.0f;
        int PitchOffset = 0;
        ModMatrix matrix;
        FpBuffer fpDataModified;

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

        // Overrides the gate signal if the preview button is held
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

        inline void OverrideMidi(FpBuffer* data)
        {
            const float cv_step = 1.0/96.0f;

            if (!midi.MidiEnabled)
                return;

            for (int i = 0; i < BUFFER_SIZE; i++)
            {
                data->Cv[0][i] = (midi.MidiNote[0]) * cv_step;
                data->Cv[1][i] = (midi.MidiNote[1]) * cv_step;
                data->Cv[2][i] = (midi.MidiNote[2]) * cv_step;
                data->Cv[3][i] = (midi.MidiNote[3]) * cv_step;

                data->Gate[0][i] = midi.MidiGate[0];
                data->Gate[1][i] = midi.MidiGate[1];
                data->Gate[2][i] = midi.MidiGate[2];
                data->Gate[3][i] = midi.MidiGate[3];

                data->GateFloat[0][i] = midi.MidiGate[0] * 1.0f;
                data->GateFloat[1][i] = midi.MidiGate[1] * 1.0f;
                data->GateFloat[2][i] = midi.MidiGate[2] * 1.0f;
                data->GateFloat[3][i] = midi.MidiGate[3] * 1.0f;

                data->Cv[0][i] += midi.MidiPitchbend[0] * midi.PitchbendRange * cv_step;
                data->Cv[1][i] += midi.MidiPitchbend[1] * midi.PitchbendRange * cv_step;
                data->Cv[2][i] += midi.MidiPitchbend[2] * midi.PitchbendRange * cv_step;
                data->Cv[3][i] += midi.MidiPitchbend[3] * midi.PitchbendRange * cv_step;
            }
        }

        inline void ApplyPitchOffset(FpBuffer* data)
        {
            const float cv_step = 1.0/96.0f;

            for (int i = 0; i < BUFFER_SIZE; i++)
            {
                data->Cv[0][i] += PitchOffset * cv_step;
                data->Cv[1][i] += PitchOffset * cv_step;
                data->Cv[2][i] += PitchOffset * cv_step;
                data->Cv[3][i] += PitchOffset * cv_step;
            }
        }
        
        
        inline void Process(DataBuffer* data)
        {
            matrix.fpData = inProcessor.ConvertToFp(data);
            matrix.fpData->CopyTo(&fpDataModified);
            OverrideIfPreview(&fpDataModified);
            OverrideMidi(&fpDataModified);
            ApplyPitchOffset(&fpDataModified);
            
            controls.SetLed(4, fpDataModified.Gate[0][0]);
            controls.SetLed(5, fpDataModified.Gate[1][0]);
            controls.SetLed(6, fpDataModified.Gate[2][0]);
            controls.SetLed(7, fpDataModified.Gate[3][0]);

            GeneratorArgs args;
            args.Bpm = 120;
            args.Data = &fpDataModified;
            args.PitchOffset = PitchOffset;
            args.GetModulationFast = [this](uint8_t slot) { return matrix.GetModulationFast(slot); };
            args.GetModulationSlow = [this](uint8_t slot) { return matrix.GetModulationSlow(slot); };
            Gen->Process(args);

            Utils::Gain(fpDataModified.Out[0], GainOut, fpDataModified.Size);
            Utils::Gain(fpDataModified.Out[1], GainOut, fpDataModified.Size);
            Utils::Gain(fpDataModified.Out[2], GainOut, fpDataModified.Size);
            Utils::Gain(fpDataModified.Out[3], GainOut, fpDataModified.Size);

            Utils::To12Bit(data->Out[0], fpDataModified.Out[0], data->Size);
            Utils::To12Bit(data->Out[1], fpDataModified.Out[1], data->Size);
            Utils::To12Bit(data->Out[2], fpDataModified.Out[2], data->Size);
            Utils::To12Bit(data->Out[3], fpDataModified.Out[3], data->Size);
        }
    };

    extern Voice voice;
}
