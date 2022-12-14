#pragma once
#include "io_buffer.h"
#include <U8g2lib.h>
#include <stdint.h>
#include <functional>
#include "fonts.h"

namespace Cyber
{
    class Menu;
    void YieldAudio();

    struct GeneratorArgs
    {
        const uint16_t Size = BUFFER_SIZE;
        FpBuffer* Data;
        float Bpm;
        int PitchOffset[4];
        std::function<float(uint8_t)> GetModulationSlow;
        std::function<float*(uint8_t)> GetModulationFast;
    };

    struct GeneratorInfo
    {
        int Version;
        const char* GeneratorId;
        const char* DisplayName;
        const char* DeveloperName;
        const char* Info;

        GeneratorInfo() 
        {
            Version = 0;
            GeneratorId = "";
            DisplayName = "";
            DeveloperName = "";
            Info = "";
        }

        GeneratorInfo(
            int version, 
            const char* generatorId, 
            const char* displayName, 
            const char* developerName, 
            const char* info) 
        {
            Version = 0;
            GeneratorId = "";
            DisplayName = "";
            DeveloperName = "";
            Info = "";
        }
    };

    class Generator
    {
    public:
        int GenIndex = 0; // used by the "OS", do not modify
        int ActiveTab = 0;
        virtual const char** GetTabs() = 0;
        virtual Menu* GetMenu() = 0;
        virtual void SetTab(int tab) = 0;
        virtual int GetModSlots() = 0;
        virtual void GetModSlotName(int idx, char* dest) = 0;
        virtual void Process(GeneratorArgs args) = 0;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) {}
        virtual void ProcessOffline() {}
        virtual ~Generator() {}

        // Your class also needs to implement these two static functions
        //static GeneratorInfo GetInfo();
        //static void SplashScreen(U8G2* display);
    };
}
