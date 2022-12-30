#pragma once
#include "generator.h"
#include "arm_math.h"
#include "utils.h"
#include "logging.h"
#include "audio_io.h"
#include "storage.h"
#include "wavReader.h"
#include "memalloc.h"
#include "modules/sampleEnvelope.h"

namespace Cyber
{
    class DRom : public SlotGenerator
    {
        enum Params
        {
            LIBRARY = 0,
            SAMPLE,
            PITCH,
            DECAY,
            KEYTRACK,
            KEYROOT,
        };

        const float fsinv = 1.0 / SAMPLERATE;

        MemoryBlock sampleData;
        Modules::SampleEnvelope env;

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
        bool keytrack = false;
        int keyRoot = 60;
        
    public:
        inline DRom() : sampleData(1024 * 160)
        {
            strcpy(TabName, "DROM");
            ParamCount = 6;
            Param[LIBRARY] = 0.0f;
            Param[SAMPLE] = 0.0f;
            Param[PITCH] = 0.5f;
            Param[DECAY] = 1.0f;
            Param[KEYTRACK] = 0.0f;
            Param[KEYROOT] = 0.469f;
            ParamUpdated();

            SetLibraryCount();
        }

        inline void SetLibraryCount()
        {
            DisableAudio disable;

            int count = Storage::GetDirCount("cyber/drom");
            LogInfof("There are %d libraries", count);
            libraryCount = count;
            SetLibrary(0);
        }

        inline void SetLibrary(int libIdx)
        {
            DisableAudio disable;

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
        }

        inline void SetSample(int sampleIdx)
        {
            DisableAudio disable;

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
        }

        inline void LoadSample()
        {
            DisableAudio disable;

            LogInfof("Loading SD Sample: %s", samplePath);
            int size = Storage::GetFileSize(samplePath);
            LogInfof("Sample is %d bytes", size);

            MemoryBlock wavData(size);
            if (wavData.GetSize() == 0)
            {
                LogInfo("insufficient RAM, cannot load!");
                return;
            }

            Storage::ReadFile(samplePath, wavData.GetPtr(), size);
            LogInfo("File read complete");
            auto info = Wav::GetWaveInfo(wavData.GetPtr(), size);
            LogInfof("Wav data, bytes per sample: %d, channels: %d, sample count: %d, sample rate: %d", 
                info.BytesPerSample, info.Channels, info.SampleCount, info.SampleRate);

            if (info.SampleCount > sampleData.GetSize() / 2)
            {
                LogInfo("Sample is too long to fit into memory");
                return;
            }

            Wav::GetWaveData<int16_t>(&info, (int16_t*)sampleData.GetPtr(), 0);
            sampleLength = info.SampleCount;
            rateScaler = info.SampleRate / (float)SAMPLERATE;
            phasor = sampleLength;

            env.SampleLength = sampleLength / inc;
            env.Update();
            LogInfo("Sample data loaded");
        }

        virtual inline void ParamUpdated() override 
        { 
            keytrack = Param[KEYTRACK] >= 0.5;
            keyRoot = Param[KEYROOT] * 127.99;
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

            env.SampleLength = sampleLength / inc;
            env.Value = Param[DECAY];
            env.Update();
        }

        virtual inline const char* GetParamName(int idx) override
        {
                 if (idx == LIBRARY) return "Library";
            else if (idx == SAMPLE) return "Sample";
            else if (idx == PITCH) return "Pitch";
            else if (idx == DECAY) return "Decay";
            else if (idx == KEYTRACK) return "Keytrack";
            else if (idx == KEYROOT) return "Key Root";
            else return "";
        }

        virtual inline void GetParamDisplay(int idx, float value, char* dest) override
        {
            if (idx == LIBRARY)
                strcpy(dest, selectedLibName);
            else if (idx == SAMPLE)
            {
                int len = strlen(selectedSampleName);
                memcpy(dest, selectedSampleName, len-4);
                dest[len-4] = 0;
            }
            else if (idx == PITCH)
            {
                float p = GetScaledParameter(idx, value);
                sprintf(dest, "%+.1f", p);
            }
            else if (idx == KEYTRACK)
            {
                strcpy(dest, Param[KEYTRACK] < 0.5 ? "Off" : "On");
            }
            else if (idx == KEYROOT)
            {
                int key = Param[KEYROOT] * 127.99;
                sprintf(dest, "%d", key);
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

            int16_t* fData = (int16_t*)sampleData.GetPtr();
            int16_t output = fData[idxA] * (1-rem) + fData[idxB] * rem;
            return output / 32768.0f;
        }

        virtual inline void Process(SlotArgs* args) override
        {
            auto g = args->Gate;
            if (!currentGate && g)
            {
                phasor = 0;
            }
            currentGate = g;
            float s = GetSampleAt(phasor);
            s = s * env.Process(g);
            args->Output = s;
            float incx = keytrack ? inc * powf(2, args->Cv * 8 - keyRoot/15.0f) : inc;
            phasor += incx;
        }

        inline float GetScaledParameter(int idx, float val = -1)
        {
            val = val == -1 ? Param[idx] : val;

            if (idx == PITCH) return (-24 + val * 48);
            return val;
        }

        inline static GeneratorInfo GetInfo()
        {
            GeneratorInfo info;
            info.DeveloperName = "Ghost Note Audio";
            info.DisplayName = "DRom";
            info.GeneratorId = "S-GNA-DRom";
            info.Info = "Sample-based drum machine with 80s sounds";
            info.Version = 1000;
            return info;
        }
    };
}
