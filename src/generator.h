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
        int PitchOffset;
        std::function<float(uint8_t)> GetModulationSlow;
        std::function<float*(uint8_t)> GetModulationFast;
    };

    struct SlotArgs
    {
        float Bpm;
        bool Gate;
        float Cv;
        float Input;
        float Output;
    };

    struct GeneratorInfo
    {
        int Version;
        const char* GeneratorId; // Max 16 characters
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
        int GenIndex; // used by the "OS", do not modify
        int ActiveTab = 0;
        virtual void GetTab(int idx, char* dest) = 0;
        virtual void SetTab(int tab) = 0;
        virtual Menu* GetMenu() = 0;
        virtual int GetModSlots() = 0;
        virtual void GetModSlotName(int idx, char* dest) = 0;
        virtual int ResolveSlot(int knobIdx) = 0;
        virtual void Process(GeneratorArgs args) = 0;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) {}
        virtual void ProcessOffline() {}
        virtual void SaveState(uint8_t* buffer, int maxLength) {}
        virtual void LoadState(uint8_t* buffer, int length) {}
        virtual ~Generator() {}

        // Your class also needs to implement these two static functions
        //static GeneratorInfo GetInfo();
        //static void SplashScreen(U8G2* display);
    };

    class SlotGenerator
    {
    public:
        int GenIndex; // used by the "OS", do not modify
        int ParamCount = 0; // Set max 16
        float Param[16] = { 0 };
        bool ParamDirty[16] = { false };
        char TabName[6] = {' ', ' ', ' ', ' ', ' ', 0};

        virtual void ParamUpdated() = 0;
        virtual const char* GetParamName(int idx) = 0;
        virtual void GetParamDisplay(int idx, float value, char* dest) = 0;
        virtual void Process(SlotArgs* args) = 0;
        virtual ~SlotGenerator() {}

        // Your class also needs to implement this static function
        //static GeneratorInfo GetInfo();
    };
}
