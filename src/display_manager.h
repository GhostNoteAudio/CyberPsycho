#pragma once

#include <i2c_driver_wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "logging.h"
#include "menus.h"

namespace Cyber
{
    extern I2CMaster& i2cMaster;

    const int ChunkSize = 16;
    const int TotalChunks = 1024 / ChunkSize;

    class DisplayManager
    {
        const int SCREEN_WIDTH = 128;
        const int SCREEN_HEIGHT = 64;
        const int OLED_RESET = -1;
        const int SCREEN_ADDRESS = 0x3C;
        Adafruit_SSD1306 display;

    public:
        inline DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 1000000)
        {
            Menus::ActiveMenu = 0;
        }

        inline void Init()
        {
            if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
            {
                Serial.println(F("SSD1306 allocation failed"));
            }
        }

        inline void Render()
        {
            Menus::ActiveMenu->Render(GetDisplay());
        }
        
        inline void Clear()
        {
            display.clearDisplay();
        }

        inline Adafruit_SSD1306* GetDisplay()
        {
            return &display;
        }

        inline void Transfer()
        {
            if (t == -1)
            {
                BeginTransfer();
                t = 0;
                return;
            }

            auto buffer = display.getBuffer();
            uint8_t *ptr = &buffer[t*ChunkSize];

            txbuf[0] = (uint8_t)0x40;
            for (int i = 0; i < ChunkSize; i++)
            {
                txbuf[i+1] = ptr[i];
            }
            
            i2cMaster.write_async(0x3C, txbuf, 1+ChunkSize, true);

            t++;
            if (t == TotalChunks)
                t = -1;
        }

    private:
        int t = 0;
        uint8_t txbufStart[7];
        uint8_t txbuf[1 + ChunkSize];

        inline void BeginTransfer()
        {
            txbufStart[0] = 0x00;
            txbufStart[1] = 0x22; // set page address
            txbufStart[2] = 0x00; // first page
            txbufStart[3] = 0xFF; // last page
            txbufStart[4] = 0x21; // set column start address
            txbufStart[5] = 0x00; // first column
            txbufStart[6] = 127; // last column
            i2cMaster.write_async(0x3C, txbufStart, 7, true);
        }
    };
}
