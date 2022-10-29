#include "kick1.h"
#include "menu.h"

namespace Cyber
{
    Kick1::Kick1()
    {
        menu.Captions[0] = "Param0";
        menu.Captions[1] = "Param1";
        menu.Captions[2] = "Param2";
        menu.Captions[3] = "Param3";

        menu.SetLength(4);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;

        ampEnv.Mode = Modules::Envelope::EnvMode::AR1shot;
        ampEnv.AttackSamples = 10;
        ampEnv.ReleaseSamples = 4400;
        ampEnv.UpdateParams();
    }


    Menu* Kick1::GetMenu()
    {
        return &menu;
    }

    void Kick1::Process(GeneratorArgs args)
    {
        for (int i = 0; i < args.Size; i++)
        {
            float sample = args.InputLeft[i];
            bool gate = args.Gate[i];
            float env = ampEnv.Process(gate);

            if (i ==0)
            {
                LogInfof("Env: %.3f Gate: %d - env stage: %d", env, (int)gate, ampEnv.Stage);
            }

            args.OutputLeft[i] = env * sample;
        }
    }

    void Kick1::ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1)
    {

    }

    void Kick1::ProcessOffline()
    {

    }
}
