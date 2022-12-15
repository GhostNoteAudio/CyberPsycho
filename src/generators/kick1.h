#pragma once

#include "generator.h"
#include "menu.h"
#include "modules/percussionEnvelope.h"

namespace Cyber
{

    class Kick1 : public Generator
    {
        const char* Tabs[4] = {"KICK", "TUNE", "", ""};
        Menu menu;
        float phasor = 0;
        bool currentGate = false;
        float adecay = 0;
        float pdecay = 0;
        float pmod = 0;
        float freq = 0;
        float boost = 0;
        float fold = 0;
        float pshape = 0;
        float ashape = 0;

        Modules::PercussionEnvelope ampEnv;
        Modules::PercussionEnvelope pitchEnv;
    public:
        Kick1();
        virtual const char** GetTabs() override { return Tabs; }
        virtual Menu* GetMenu() override;
        virtual inline void SetTab(int tab) override;
        virtual int GetModSlots();
        virtual void GetModSlotName(int idx, char* dest);
        virtual int ResolveSlot(int knobIdx);
        virtual void Process(GeneratorArgs args) override;

    private:
        void UpdateAll(GeneratorArgs args);
        float GetScaledParameter(int idx, float modulation=0);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Kick One";
            info.GeneratorId = "GNA-Kick1";
            info.Info = "X0X-esque kick drum synth.";
            info.Version = 1000;
            return info;
        }

        inline static void SplashScreen(U8G2* display)
        {
            display->clearDisplay();
            display->setDrawColor(1);
            display->setFont(BIG_FONT);
            display->setCursor(4, 36);
            display->print("Kick 1");
        }
    };

}