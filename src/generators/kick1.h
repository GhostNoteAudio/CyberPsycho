#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/envelope.h"

namespace Cyber
{

    class Kick1 : public Generator
    {
        Menu menu;
        Modules::Envelope ampEnv;
    public:
        Kick1();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) override;
        virtual void ProcessOffline() override;

        static void SplashScreen(Adafruit_GFX* display)
        {
            display->fillScreen(SH110X_BLACK);
            display->setTextColor(SH110X_WHITE);
            display->setFont(&AtlantisInternational_jen08pt7b);
            display->setTextSize(2);
            display->setCursor(10, 32);
            display->print("Kick 1");
        }
    };

}