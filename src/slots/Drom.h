#pragma once
#include "generator.h"
#include "arm_math.h"
#include "utils.h"
#include "logging.h"
#include "audio_io.h"
#include "storage.h"
#include "wavReader.h"

namespace Cyber
{
    uint8_t WavData[1024*32];
    float SampleData[1024*32];

    class DRom : public SlotGenerator
    {
        enum Params
        {
            LIBRARY = 0,
            SAMPLE = 1,
            PITCH = 2,
        };

        const float fsinv = 1.0 / SAMPLERATE;

        int libraryCount = 0;
        int sampleCount = 0;
        int selectedLib = 0;
        int selectedSample = 0;
        char selectedLibName[16];
        char selectedSampleName[16];
        char samplePath[64];

        int sampleLength = 0;
        float rateScaler = 1;
        float phasor = 0;
        float inc = 0;
        bool currentGate = false;
        
    public:
        inline DRom()
        {
            strcpy(TabName, "DROM");
            ParamCount = 3;
            Param[0] = 0.0f;
            Param[1] = 0.0f;
            Param[2] = 0.5f;
            ParamUpdated();

            SetLibraryCount();
        }

        inline void SetLibraryCount()
        {
            audio.StopProcessing();
            delayMicroseconds(100);

            int count = Storage::GetDirCount("cyber/drom");
            LogInfof("There are %d libraries", count);
            libraryCount = count;
            SetLibrary(0);

            audio.StartProcessing();
            delayMicroseconds(100);
        }

        inline void SetLibrary(int libIdx)
        {
            audio.StopProcessing();
            delayMicroseconds(100);

            selectedLib = libIdx;

            auto libName = Storage::GetName("cyber/drom", selectedLib, true);
            LogInfof("Selected lib: %s", libName);
            strcpy(selectedLibName, libName);

            char path[64];
            strcpy(path, "cyber/drom/");
            strcat(path, selectedLibName);
            sampleCount = Storage::GetFileCount(path);
            LogInfof("There are %d samples in this library", sampleCount);
            SetSample(0);
            
            audio.StartProcessing();
            delayMicroseconds(100);
        }

        inline void SetSample(int sampleIdx)
        {
            audio.StopProcessing();
            delayMicroseconds(100);

            selectedSample = sampleIdx;
            char path[64];
            strcpy(path, "cyber/drom/");
            strcat(path, selectedLibName);

            auto sampleName = Storage::GetName(path, selectedSample, false);
            LogInfof("Selected sample: %s", sampleName);
            strcpy(selectedSampleName, sampleName);

            strcat(path, "/");
            strcat(path, sampleName);
            strcpy(samplePath, path);
            LogInfof("Selected SamplePath: %s", samplePath);
            LoadSample();

            audio.StartProcessing();
            delayMicroseconds(100);
        }

        inline void LoadSample()
        {
            audio.StopProcessing();
            delayMicroseconds(100);

            LogInfof("Loading SD Sample: %s", samplePath);
            int size = Storage::GetFileSize(samplePath);
            LogInfof("Sample is %d bytes", size);

            if (size > 1024*32)
            {
                LogInfo("File is too big, cannot load!");
                return;
            }

            Storage::ReadFile(samplePath, WavData, sizeof(WavData));
            LogInfo("File read complete");
            auto info = Wav::GetWaveInfo(WavData, size);
            LogInfof("Wav data, bytes per sample: %d, channels: %d, sample count: %d, sample rate: %d", 
                info.BytesPerSample, info.Channels, info.SampleCount, info.SampleRate);
            Wav::GetWaveData(&info, SampleData, 0);
            LogInfo("ping2");
            sampleLength = info.SampleCount;
            LogInfo("ping3");
            rateScaler = info.SampleRate / (float)SAMPLERATE;
            LogInfo("ping4");
            phasor = sampleLength;
            LogInfo("Sample data loaded");

            audio.StartProcessing();
            delayMicroseconds(100);
            LogInfo("ping5");
        }

        virtual inline void ParamUpdated() override 
        { 
            inc = powf(2, 4*Param[PITCH]-2) * rateScaler;
            int newSelectedLib = (int)(Param[LIBRARY] * (libraryCount - 0.01));
            int newSelectedSample = (int)(Param[SAMPLE] * (sampleCount - 0.01));

            if (newSelectedLib != selectedLib)
            {
                SetLibrary(newSelectedLib);
            }
            else if (newSelectedSample != selectedSample)
            {
                SetSample(newSelectedSample);
            }
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == LIBRARY) return "Library";
            else if (idx == SAMPLE) return "Sample";
            else if (idx == PITCH) return "Pitch";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == LIBRARY)
                strcpy(dest, selectedLibName);
            else if (idx == SAMPLE)
                strcpy(dest, selectedSampleName);
            else if (idx == PITCH)
            {
                float p = GetScaledParameter(idx, value);
                sprintf(dest, "%+.1f", p);
            }
            else
            {
                sprintf(dest, "%.1f", value * 100);
            }
        }

        inline float GetSampleAt(float index)
        {
            int idxA = (int)index;
            int idxB = idxA + 1;
            float rem = index - idxA;

            if (idxB >= sampleLength)
                return 0;

            return SampleData[idxA] * (1-rem) + SampleData[idxB] * rem;
        }

        virtual inline void Process(SlotArgs* args) override
        {
            auto g = args->Gate;
            if (!currentGate && g)
            {
                phasor = 0;
            }
            currentGate = g;

            args->Output = GetSampleAt(phasor);
            phasor += inc;
        }

        inline float GetScaledParameter(int idx, float val = -1)
        {
            val = val == -1 ? Param[idx] : val;

            if (idx == PITCH) return (-24 + val * 48);
            return 0;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "DRom";
            info.GeneratorId = "Slot-GNA-DRom";
            info.Info = "Sample-based drum machine with 80s sounds";
            info.Version = 1000;
            return info;
        }
    };
}
