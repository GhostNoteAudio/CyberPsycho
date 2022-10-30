#pragma once
#include "io_buffer.h"
#include "Adafruit_GFX.h"
#include <stdint.h>
#include <functional>

namespace Cyber
{
    class Menu;
    void YieldAudio();

    struct GeneratorArgs
    {
        const uint16_t Size = BUFFER_SIZE;
        bool StereoIn;
        bool StereoOut;
        float Bpm;

        float* InputLeft;
        float* InputRight;
        float* OutputLeft;
        float* OutputRight;
        float* Cv;
        bool* Gate;
        
        std::function<float*(int)> GetModulation;
    };

    struct GeneratorInfo
    {
        const int Version;
        const char* GeneratorId;
        const char* DisplayName;
        const char* DeveloperName;
        const char* Info;
    };

    class Generator
    {
    public:
        virtual Menu* GetMenu() = 0;
        virtual void Process(GeneratorArgs args) = 0;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) {}
        virtual void ProcessOffline() {}
        virtual ~Generator() {}
    };
}
