#pragma once
#include <U8g2lib.h>
#include "generator.h"

namespace Cyber
{
    const int MAX_GEN_COUNT = 50;

    class GeneratorRegistry
    {
        int SlotGenCount;
        std::function<SlotGenerator*(void)> SlotGenFactories[MAX_GEN_COUNT];
        GeneratorInfo SlotGenInfo[MAX_GEN_COUNT];

        int GenCount;
        std::function<Generator*(void)> GenFactories[MAX_GEN_COUNT];
        GeneratorInfo GenInfo[MAX_GEN_COUNT];
        std::function<void(U8G2*)> GenSplashScreenBuilders[MAX_GEN_COUNT];

    public:
        // Slots
        
        inline int GetSlotGenCount() { return SlotGenCount; }

        template<class TSlotGen>
        inline void AddSlotGen()
        {
            SlotGenFactories[SlotGenCount] = []{return new TSlotGen();};
            SlotGenInfo[SlotGenCount] = TSlotGen::GetInfo();
            SlotGenCount++;
        }

        inline void DeleteSlotGenInstance(SlotGenerator* gen)
        {
            delete gen;
        }

        inline int GetSlotGenIndexById(const char* slotGenId)
        {
            for (int i = 0; i < SlotGenCount; i++)
            {
                if (strcmp(SlotGenInfo[i].GeneratorId, slotGenId) == 0)
                {
                    return i;
                }
            }
            return -1;
        }

        inline SlotGenerator* CreateSlotGenInstance(int idx)
        {
            auto instance = SlotGenFactories[idx]();
            instance->GenIndex = idx;
            return instance;
        }

        // Generators

        inline int GetGenCount() { return GenCount; }
        inline std::function<void(U8G2*)> GetSplashScreenBuilder(int idx) { return GenSplashScreenBuilders[idx]; }

        template<class TGen>
        inline void AddGen()
        {
            GenFactories[GenCount] = []{return new TGen();};
            GenInfo[GenCount] = TGen::GetInfo();
            GenSplashScreenBuilders[GenCount] = [](U8G2* display){TGen::SplashScreen(display);};
            GenCount++;
        }

        inline void DeleteGenInstance(Generator* gen)
        {
            delete gen;
        }

        inline int GetGenIndexById(const char* genId)
        {
            for (int i = 0; i < GenCount; i++)
            {
                if (strcmp(GenInfo[i].GeneratorId, genId) == 0)
                {
                    return i;
                }
            }
            return -1;
        }

        inline Generator* CreateGenInstance(int idx)
        {
            auto instance = GenFactories[idx]();
            instance->GenIndex = idx;
            return instance;
        }
    };

    extern GeneratorRegistry generatorRegistry;
}
