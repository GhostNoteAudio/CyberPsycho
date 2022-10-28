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
    }


    Menu* Kick1::GetMenu()
    {
        return &menu;
    }

    void Kick1::Process(GeneratorArgs args)
    {
        for (int i = 0; i < args.Size; i++)
        {
            args.OutputLeft->Data[i] = args.InputLeft->Data[i];
        }
    }

    void Kick1::ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1)
    {

    }

    void Kick1::ProcessOffline()
    {

    }
}
