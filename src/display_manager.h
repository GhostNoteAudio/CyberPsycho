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
            //display.display();
            //return;

            int _page_start_offset = 2; // for the SH1106
            uint8_t dc_byte = 0x40;
            uint8_t pages = ((SCREEN_HEIGHT + 7) / 8);
            uint8_t bytes_per_page = SCREEN_WIDTH;

            auto buffer = display.getBuffer();
            
            if (p == 0 && page_bytes_remaining == bytes_per_page)
            {
                ptr = &buffer[0];
            }

            
            if (page_bytes_remaining == bytes_per_page)
            {
                txbufStart[0] = 0x00;
                txbufStart[1] = (uint8_t)(SH110X_SETPAGEADDR + p);
                txbufStart[2] = (uint8_t)(0x10 + ((0 + _page_start_offset) >> 4));
                txbufStart[3] = (uint8_t)((0 + _page_start_offset) & 0xF);
                i2cMaster.write_async(SCREEN_ADDRESS, txbufStart, 4, true);
                while(!i2cMaster.finished()) {}
            }
            
            uint8_t to_write = min(page_bytes_remaining, (uint8_t)ChunkSize);

            txbuf[0] = dc_byte;
            for (int i = 0; i < to_write; i++)
                txbuf[i+1] = ptr[i];

            //i2c_dev->write(ptr, to_write, true, &dc_byte, 1);
            i2cMaster.write_async(SCREEN_ADDRESS, txbuf, to_write+1, true);
            while(!i2cMaster.finished()) {}

            ptr += to_write;
            page_bytes_remaining -= to_write;
            

            if (page_bytes_remaining == 0)
            {
                p++;
                page_bytes_remaining = bytes_per_page;
            }
            
            if (p == pages)
                p = 0;
            
        }

    private:
        int p = 0;
        uint8_t *ptr;
        uint8_t page_bytes_remaining = SCREEN_WIDTH;
        uint8_t txbufStart[4];
        uint8_t txbuf[300];
    };
}
