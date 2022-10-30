#pragma once

#include "generator.h"
#include "menu.h"
#include <fonts/font3.h>
#include "modules/biquad.h"

namespace Cyber
{

    class Superwave : public Generator
    {
        Menu menu;
        float pitch = 48;
        float gainAdjust = 1.0;
        int phasor[7] = {0};
        float volumes[7] = {0};
        Modules::Biquad biq;
        
    public:
        Superwave();
        virtual Menu* GetMenu() override;
        virtual void Process(GeneratorArgs args) override;
        void Reset(float pitchHz = -1);

    private:
        float GetScaledParameter(int idx);

    public:
        inline static const char* GetName() { return "Superwave"; }
        inline static void SplashScreen(Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setTextColor(SH110X_WHITE);
            display->setFont(&AtlantisInternational_jen08pt7b);
            display->setTextSize(2);
            display->setCursor(0, 32);
            display->print("Superwave");
        }
    };

}