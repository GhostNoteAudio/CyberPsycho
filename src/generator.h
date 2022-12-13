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
        bool Stereo;
        float Bpm;

        float* InputLeft;
        float* InputRight;
        float* OutputLeft;
        float* OutputRight;
        float* Cv;
        bool* Gate;
        
        std::function<float(uint8_t)> GetModulationSlow;
        std::function<float*(uint8_t)> GetModulationFast;
    };

    struct GeneratorInfo
    {
        int Version;
        bool InsertEffect;
        bool StereoInSupport;
        bool StereoOutSupport;
        int ModSlotCount;
        const char* GeneratorId;
        const char* DisplayName;
        const char* DeveloperName;
        const char* Info;

        GeneratorInfo() 
        {
            Version = 0;
            InsertEffect = false;
            GeneratorId = "";
            DisplayName = "";
            DeveloperName = "";
            Info = "";
        }

        GeneratorInfo(
            int version, 
            bool insertEffect,
            const char* generatorId, 
            const char* displayName, 
            const char* developerName, 
            const char* info) 
        {
            Version = 0;
            InsertEffect = insertEffect;
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
        virtual Menu* GetMenu() = 0;
        virtual void Process(GeneratorArgs args) = 0;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) {}
        virtual void ProcessOffline() {}
        virtual ~Generator() {}

        // Your class also needs to implement these two static functions
        //static GeneratorInfo GetInfo();
        //static void SplashScreen(U8G2* display);
    };
}
