#include "storage.h"
#include "cyberpsycho.h"

namespace Cyber
{
    struct NoInterrupt
    {
        NoInterrupt()
        {
            //cli();
        }

        ~NoInterrupt()
        {
            //sei();
        }
    };

    namespace Storage
    {
        SdFat sd;
        char filePathBuffer[256];

        bool InitStorage()
        {
            NoInterrupt ni;
            if (!sd.begin(PIN_CS_SD, 2000000))
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
            NoInterrupt ni;
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
            NoInterrupt ni;
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

        bool CreateFolder(const char* dirPath)
        {
            NoInterrupt ni;
            return sd.mkdir(dirPath);
        }

        int GetFileCount(const char* dirPath)
        {
            NoInterrupt ni;
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
                count++;
                file.close();
            }
            if (dir.getError()) 
            {
                Serial.println("Failed to list directory");
                return -1;
            }
            return count;
        }

        const char* GetFilePath(const char* dirPath, int index)
        {
            NoInterrupt ni;
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
                if (count == index)
                {
                    strcpy(filePathBuffer, dirPath);
                    filePathBuffer[strlen(dirPath)] = '/';
                    file.getName(&filePathBuffer[strlen(dirPath) + 1], sizeof(filePathBuffer));
                    file.close();
                    return filePathBuffer;
                }
                file.close();
                count++;
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
            NoInterrupt ni;
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
            NoInterrupt ni;
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

        void ReadFile(const char* filePath, uint8_t* data, int maxDataLen)
        {
            NoInterrupt ni;
            SdFile myFile;
            if (!myFile.open(filePath, O_RDONLY))
            {
                sd.errorPrint("Failed to open file");
                return;
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
        }

        void SaveGlobalState()
        {
            LogInfo("Trying to save state to SD card")
            audio.StopProcessing();
            delayMicroseconds(100);

            bool result = true;

            bool dirExists = DirExists("cyber/state");
            if (!dirExists)
                result = CreateFolder("cyber/state");

            if (!result) { LogInfo("Failed to create folder cyber/state"); return; }
            LogInfo("Ping1");

            result = WriteFile("cyber/state/global.bin", (uint8_t*)Menus::globalMenu.Values, sizeof(float) * MENU_MAX_SIZE);
            if (!result) { LogInfo("Failed to write global state"); return; }
            LogInfo("Ping2");

            result = WriteFile("cyber/state/calibrate.bin", (uint8_t*)Menus::calibrateMenu.Values, sizeof(float) * MENU_MAX_SIZE);
            if (!result) { LogInfo("Failed to write calibrate state"); return; }
            LogInfo("Ping3");

            result = WriteFile("cyber/state/gen.bin", (uint8_t*)voice.Gen->GetMenu()->Values, sizeof(float) * MENU_MAX_SIZE);
            if (!result) { LogInfo("Failed to write generator state"); return; }
            LogInfo("Ping4");

            result = WriteFile("cyber/state/matrix.bin", (uint8_t*)voice.matrix.Routes, sizeof(voice.matrix.Routes));
            if (!result) { LogInfo("Failed to write matrix state"); return; }
            LogInfo("Ping5");

            LogInfo("Successfully saved state to SD card");

            auto saveOverlay = [](U8G2* display)
            {
                display->setFont(DEFAULT_FONT);
                display->setDrawColor(0);
                display->drawBox(20, 20, 128-40, 24);
                display->setDrawColor(1);
                display->drawFrame(20, 20, 128-40, 24);
                int w = display->getStrWidth("Saved");
                display->setCursor(64 - w/2, 35);
                display->print("Saved");
            };
            displayManager.SetOverlay(saveOverlay, 1000);

            audio.StartProcessing();
        }

        void SavePreset(int slot)
        {

        }
    }
}
