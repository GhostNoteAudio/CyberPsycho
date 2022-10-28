#pragma once
#include "io_buffer.h"
#include "Adafruit_GFX.h"
#include <stdint.h>
#include <functional>

namespace Cyber
{
    class Menu;

    struct GeneratorArgs
    {
        const uint16_t Size = BUFFER_SIZE;
        bool StereoIn;
        bool StereoOut;
        float Bpm;

        AudioBuffer* InputLeft;
        AudioBuffer* InputRight;
        AudioBuffer* OutputLeft;
        AudioBuffer* OutputRight;
        AudioBuffer* Gate;
        AudioBuffer* Cv;
        
        std::function<AudioBuffer*(int)> GetModulation;
    };

    class Generator
    {
    public:
        virtual Menu* GetMenu() = 0;
        virtual void Process(GeneratorArgs args) = 0;
        virtual void ProcessMidi(uint8_t type, uint8_t data0, uint8_t data1) = 0;
        virtual void ProcessOffline() = 0;
    };

    void YieldAudio();
    void RegisterGenerator(std::function<Generator*(void)> factory, const char* name, std::function<void(Adafruit_GFX*)> splashScreenBuilder);
}
