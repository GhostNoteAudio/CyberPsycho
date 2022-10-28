#pragma once

#include "generator.h"
#include "menu.h"

namespace Cyber
{

    class Kick1 : public Generator
    {
        Menu menu;
    public:
        Kick1();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) override;
        virtual void ProcessOffline() override;
    };

}