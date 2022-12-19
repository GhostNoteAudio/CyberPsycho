#pragma once

#include <SdFat.h>
#include <stdint.h>

namespace Cyber
{
    namespace Storage
    {
        extern SdFat sd;
        extern char filePathBuffer[256];

        bool InitStorage();
        bool DirExists(const char* dirPath);
        bool FileExists(const char* filePath);
        bool CreateFolder(const char* dirPath);
        int GetDirCount(const char* dirPath);
        int GetFileCount(const char* dirPath);
        int GetCount(const char* dirPath, bool dirs);
        const char* GetName(const char* dirPath, int index, bool dirs);
        bool WriteFile(const char* filePath, uint8_t* data, int dataLen);
        int GetFileSize(const char* filePath);
        void ReadFile(const char* filePath, uint8_t* data, int maxDataLen);

        void SaveGlobalState();
        void SavePreset(int slot);
    }
}
