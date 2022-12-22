#pragma once
#include "stdint.h"
#include "io_buffer.h"
#include "mod_source.h"
#include "menu.h"

namespace Cyber
{
    class Voice;

    struct ModRouting
    {
        ModSource Source;
        uint8_t Slot;
        float Amount;
    };

    const int ModRouteCount = 16;

    class ModMatrix
    {
        Menu menu;
        uint8_t activeRoute = 0;

        float OutputBuffer[BUFFER_SIZE];
        float OutputValue;

    public:
        ModRouting Routes[ModRouteCount];
        int LastUpdatedRoute = -1;
        FpBuffer* fpData;

        inline ModMatrix() { InitMenu(); }
        inline Menu* GetMenu() { return &menu; }
        void InitMenu();
        void UpdateMenuDisplay();
        void UpdateRoute(ModSource source, int slot, float value);
        float GetModulationSlow(uint8_t slot);
        float* GetModulationFast(uint8_t slot);
        void Cleanup();
        void Reset();

    private:
        float* GetSourceBuffer(ModSource source);
        void AddRouteSlow(int idx);
        void AddRouteFast(int idx);
    };
}
