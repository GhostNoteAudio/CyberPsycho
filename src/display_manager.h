#pragma once

#include <i2c_driver_wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#include "logging.h"
#include "menus.h"

namespace Cyber
{
    extern I2CMaster& i2cMaster;

    const int ChunkSize = 16;

    class DisplayManager
    {
        const int SCREEN_WIDTH = 128;
        const int SCREEN_HEIGHT = 64;
        const int OLED_RESET = -1;
        const int SCREEN_ADDRESS = 0x3C;
        Adafruit_SH1106G display;

    public:
        inline DisplayManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 1000000)
        {
            Menus::ActiveMenu = 0;
        }

        inline void Init()
        {
            if (!display.begin(SCREEN_ADDRESS)) 
            {
                Serial.println(F("Display allocation failed"));
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

        inline Adafruit_SH1106G* GetDisplay()
        {
            return &display;
        }

        inline void Transfer()
        {
            int _page_start_offset = 2; // for the SH1106
            uint8_t DC_BYTE = 0x40;
            uint8_t pageCount = ((SCREEN_HEIGHT + 7) / 8);
            uint8_t bytesPerPage = SCREEN_WIDTH;
            auto buffer = display.getBuffer();
            
            // page 0, byte 0; reset the ptr to start of buffer;
            if (p == 0 && pageBytesRemaining == bytesPerPage)
            {
                ptr = &buffer[0];
            }

            // New page boundary, issue start commands for that page
            if (pageBytesRemaining == bytesPerPage)
            {
                txbufStart[0] = 0x00;
                txbufStart[1] = (uint8_t)(SH110X_SETPAGEADDR + p);
                txbufStart[2] = (uint8_t)(0x10 + ((0 + _page_start_offset) >> 4));
                txbufStart[3] = (uint8_t)((0 + _page_start_offset) & 0xF);
                i2cMaster.write_async(SCREEN_ADDRESS, txbufStart, 4, true);
                while(!i2cMaster.finished()) {}
            }
            
            uint8_t to_write = min(pageBytesRemaining, (uint8_t)ChunkSize);

            txbuf[0] = DC_BYTE;
            for (int i = 0; i < to_write; i++)
                txbuf[i+1] = ptr[i];

            i2cMaster.write_async(SCREEN_ADDRESS, txbuf, to_write+1, true);
            while(!i2cMaster.finished()) {}

            ptr += to_write;
            pageBytesRemaining -= to_write;

            if (pageBytesRemaining == 0)
            {
                p = (p+1) % pageCount;
                pageBytesRemaining = bytesPerPage;
            }
        }

    private:
        int p = 0;
        uint8_t pageBytesRemaining = SCREEN_WIDTH;
        uint8_t *ptr;

        uint8_t txbufStart[4];
        uint8_t txbuf[ChunkSize+1];
    };
}
