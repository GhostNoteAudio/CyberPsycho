#include "storage.h"
#include "cyberpsycho.h"
#include "menus.h"

namespace Cyber
{
    namespace Storage
    {
        SdFat sd;
        char filePathBuffer[256];

        bool InitStorage()
        {
            if (!sd.begin(PIN_CS_SD, 24000000))
            {
                LogInfo("SD Card initialization FAILED.")
                sd.initErrorPrint(&Serial);
                return false;
            }
            LogInfo("SD Card initialization done.")
            return true;
        }

        bool DirExists(const char* dirPath)
        {
            if (!sd.exists(dirPath))
                return false;
            
            SdFile dir;
            if (!dir.open(dirPath))
            {
                sd.errorPrint("Failed to open directory");
                return false;
            }

            return dir.isDir();
        }

        bool FileExists(const char* filePath)
        {
            if (!sd.exists(filePath))
                return false;
            
            SdFile file;
            if (!file.open(filePath, O_RDONLY))
            {
                sd.errorPrint("Failed to open file");
                return false;
            }

            return file.isFile();
        }

        bool CreateDirectory(const char* dirPath)
        {
            LogInfof("Creating directory %s", dirPath);

            bool dirExists = DirExists(dirPath);
            if (dirExists) 
                return true;

            return sd.mkdir(dirPath);
        }

        int GetDirCount(const char* dirPath)
        {
            return GetCount(dirPath, true);
        }

        int GetFileCount(const char* dirPath)
        {
            return GetCount(dirPath, false);
        }

        int GetCount(const char* dirPath, bool dirs)
        {
            SdFile dir, file;
            if (!dir.open(dirPath))
            {
                sd.errorPrint("Failed to open directory");
                return -1;
            }

            if (!dir.isDir())
                return -1;

            int count = 0;
            while (file.openNext(&dir, O_RDONLY)) 
            {
                if (file.isDir())
                {
                    if (dirs) count++;
                }
                else // isFile
                {
                    if (!dirs) count++;
                }

                file.close();
            }
            if (dir.getError()) 
            {
                Serial.println("Failed to list directory");
                return -1;
            }
            return count;
        }

        const char* GetName(const char* dirPath, int index, bool dirs)
        {
            strcpy(filePathBuffer, "");

            SdFile dir, file;
            if (!dir.open(dirPath))
            {
                sd.errorPrint("Failed to open directory");
                return filePathBuffer;
            }

            if (!dir.isDir())
            {
                return filePathBuffer;
            }

            int count = 0;
            while (file.openNext(&dir, O_RDONLY)) 
            {
                if (file.isDir() && dirs)
                {
                    if (count == index)
                    {
                        file.getName(filePathBuffer, sizeof(filePathBuffer));
                        file.close();
                        return filePathBuffer;
                    }
                    count++;
                }
                else if (!file.isDir() && !dirs)
                {
                    if (count == index)
                    {
                        file.getName(filePathBuffer, sizeof(filePathBuffer));
                        file.close();
                        return filePathBuffer;
                    }
                    count++;
                }

                file.close();
            }

            if (dir.getError()) 
            {
                Serial.println("Failed to list directory");
                return filePathBuffer;
            }

            return filePathBuffer;
        }

        bool WriteFile(const char* filePath, uint8_t* data, int dataLen)
        {
            SdFile myFile;
            if (!myFile.open(filePath, O_RDWR | O_CREAT | O_TRUNC)) 
            {
                Serial.println("open failed");
                return false;
            }

            auto result = myFile.write(data, dataLen);
            myFile.close();
            return (int)result == dataLen;
        }

        int GetFileSize(const char* filePath)
        {
            SdFile myFile;
            if (!myFile.open(filePath, O_RDONLY))
            {
                sd.errorPrint("Failed to open file");
                return -1;
            }
            int output = myFile.available();
            myFile.close();
            return output;
        }

        bool ReadFile(const char* filePath, uint8_t* data, int maxDataLen)
        {
            SdFile myFile;
            if (!myFile.open(filePath, O_RDONLY))
            {
                sd.errorPrint("Failed to open file");
                return false;
            }

            int read_count = 0;
            while(true)
            {
                int b = myFile.read();
                if (b == -1)
                    break;
                data[read_count] = b;
                read_count++;
                if (read_count >= maxDataLen)
                    break;
            }

            myFile.close();
            return true;
        }

        void LoadPreset(const char* filepath, bool showOverlay)
        {
            DisableAudio disable;
            uint8_t data[1024];

            LogInfof("Loading preset from file %s", filepath);
            int result = ReadFile(filepath, data, 1024);
            if (!result) { LogInfo("Failed to load preset"); return; }
            
            char genId[16];
            int genIdSize = 16;
            int matrixSize = sizeof(voice.matrix.Routes);
            int menuDataSize = sizeof(float) * MENU_MAX_SIZE;
            int stateSize = 512;

            uint8_t* genIdPtr = &data[0];
            uint8_t* matrixPtr = &data[genIdSize];
            uint8_t* menuDataPtr = &data[genIdSize+matrixSize];
            uint8_t* stateDataPtr = &data[genIdSize+matrixSize+menuDataSize];

            strcpy(genId, (char*)genIdPtr);

            LogInfof("Loading generator %s", genId);
            voice.SetGenerator(generatorRegistry.GetGenIndexById(genId));
            LogInfo("Generator Loaded");

            memcpy(voice.matrix.Routes, matrixPtr, matrixSize);
            voice.Gen->LoadState(stateDataPtr, stateSize);
            memcpy(voice.Gen->GetMenu()->Values, menuDataPtr, menuDataSize);
            LogInfo("Preset Loaded");
            if (showOverlay)
                displayManager.SetOverlay(loadOverlay, 1000);
        }

        void SavePreset(const char* dir, const char* filename, bool showOverlay)
        {
            DisableAudio disable;

            uint8_t data[1024];            
            CreateDirectory(dir);

            int written = 0;
            int size = 0;

            auto genId = generatorRegistry.GetGenInfo(voice.Gen->GenIndex).GeneratorId;
            memset(data, 0, 16);
            strcpy((char*)data, genId);
            written += 16;

            size = sizeof(voice.matrix.Routes);
            memcpy(&data[written], voice.matrix.Routes, size);
            written += size;

            size = sizeof(float) * MENU_MAX_SIZE;
            memcpy(&data[written], voice.Gen->GetMenu()->Values, size);
            written += size;

            size = 512;
            memset(&data[written], 0, size);
            voice.Gen->SaveState(&data[written], size);
            written += size;
            
            strcpy(filePathBuffer, dir);
            strcat(filePathBuffer, "/");
            strcat(filePathBuffer, filename);
            LogInfof("Saving preset (%d bytes) to %s", written, filePathBuffer);
            bool result = WriteFile(filePathBuffer, data, written);
            if (!result) { LogInfo("Failed to write preset"); return; }
            LogInfo("Preset saved");
            if (showOverlay)
                displayManager.SetOverlay(saveOverlay, 1000);
        }

        void LoadGlobalState()
        {
            LogInfo("Trying to load state from SD card")
            DisableAudio disable;

            bool result = false;

            bool exists = DirExists("cyber/state");
            if (!exists) { return; }
            LogInfo("Ping1");

            uint8_t data[512];

            result = ReadFile("cyber/state/global.bin", data, 512);
            if (!result) { LogInfo("Failed to read global state"); return; }
            memcpy(Menus::globalMenu.Values, data, sizeof(float) * MENU_MAX_SIZE);
            LogInfo("Ping2");

            result = ReadFile("cyber/state/calibrate.bin", data, 512);
            if (!result) { LogInfo("Failed to read calibrate state"); return; }
            memcpy(Menus::calibrateMenu.Values, data, sizeof(float) * MENU_MAX_SIZE);
            LogInfo("Ping3");

            LoadPreset("cyber/state/preset.bin");

            LogInfo("Successfully loaded state from SD card");
        }

        void SaveGlobalState()
        {
            LogInfo("Trying to save state to SD card")
            DisableAudio disable;

            bool result = true;

            result = CreateDirectory("cyber/state");
            if (!result) { LogInfo("Failed to create folder cyber/state"); return; }
            LogInfo("Ping1");

            result = WriteFile("cyber/state/global.bin", (uint8_t*)Menus::globalMenu.Values, sizeof(float) * MENU_MAX_SIZE);
            if (!result) { LogInfo("Failed to write global state"); return; }
            LogInfo("Ping2");

            result = WriteFile("cyber/state/calibrate.bin", (uint8_t*)Menus::calibrateMenu.Values, sizeof(float) * MENU_MAX_SIZE);
            if (!result) { LogInfo("Failed to write calibrate state"); return; }
            LogInfo("Ping3");

            SavePreset("cyber/state", "preset.bin");
            LogInfo("Successfully saved state to SD card");

            displayManager.SetOverlay(saveOverlay, 1000);
        }
    }
}
