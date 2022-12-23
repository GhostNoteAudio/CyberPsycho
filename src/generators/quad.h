#pragma once

#include "generator.h"
#include "menu.h"

namespace Cyber
{
    class Quad : public Generator
    {
        enum Input
        {
            INPUT_OFF = 0,
            INPUT_EXT,
            INPUT_PREV
        };

        bool selectionModeActive = false;
        int selectedGen = 0;
        Menu menu;
        SlotGenerator* Slots[4] = {0};
        Input Inputs[4] = {INPUT_EXT, INPUT_EXT, INPUT_EXT, INPUT_EXT};
        float GainInDb[4] = {0.5, 0.5, 0.5, 0.5};
        float GainOutDb[4] = {0.5, 0.5, 0.5, 0.5};
        
    public:
        Quad();
        virtual void GetTab(int idx, char* dest) override;
        virtual void SetTab(int tab) override;
        virtual Menu* GetMenu() override;
        virtual int GetModSlots() override;
        virtual void GetModSlotName(int idx, char* dest) override;
        virtual int ResolveSlot(int knobIdx) override;
        virtual void Process(GeneratorArgs args) override;
        virtual void SaveState(uint8_t* buffer, int maxLength) override;
        virtual void LoadState(uint8_t* buffer, int length) override;
        
    private:
        void RenderSlotSelectionMenu(U8G2* display);
        void UpdateMenuSections();
        void UpdateCaptionsValues();
        void SetSlotGen(int slot, int genId);

    public:
        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "Quad";
            info.GeneratorId = "GNA-Quad";
            info.Info = "Loads four slot effects.";
            info.Version = 1000;
            return info;
        }

        inline static void SplashScreen(U8G2* display)
        {
            display->clearDisplay();
            display->setDrawColor(1);
            display->setFont(BIG_FONT);
            display->setCursor(4, 36);
            display->print("Quad");
        }
    };
}
