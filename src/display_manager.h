#pragma once

#include <functional>
#include <i2c_driver_wire.h>
#include <Adafruit_SH110X.h>
#include <U8g2lib.h>
#include "logging.h"
#include "menus.h"
#include "fonts.h"
#include "voice.h"

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
        Adafruit_SH1106G display; // For sending
        U8G2_SH1106_128X64_NONAME_F_2ND_HW_I2C display2; // For drawing
        
        std::function<void(U8G2*)> overlayCallback;
        uint overlayExpiration = 0;

    public:
        Menu* ActiveMenu;

        inline DisplayManager() : 
            display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 1000000), 
            display2(U8G2_R0, U8X8_PIN_NONE)
        {
            ActiveMenu = 0;
        }

        inline void Init()
        {
            if (!display2.begin()) 
            {
                Serial.println(F("Display2 allocation failed"));
            }
            display2.setFont(DEFAULT_FONT);
            display2.setCursor(0, 10);
            
            if (!display.begin(SCREEN_ADDRESS)) 
            {
                Serial.println(F("Display allocation failed"));
            }

            ActiveMenu = &Menus::initMenu;
        }

        inline void SetOverlay(std::function<void(U8G2*)> callback, int timeoutMillis)
        {
            overlayCallback = callback;
            overlayExpiration = millis() + timeoutMillis;
        }

        inline void RenderHeader()
        {
            int activeTab = voice.Gen->ActiveTab;
            auto display = GetDisplay();
            display->setDrawColor(1);
            display->setFont(SMALL_FONT);
            char tabval[16];

            YieldAudio();
            int offsets[] = {0, 23, 128-47, 128-24};
            
            for (int i = 0; i < 4; i++)
            {
                voice.Gen->GetTab(i, tabval);
                if (strlen(tabval) == 0)
                    continue;

                if (i == activeTab)
                    display->drawBox(offsets[i], 0, 24, 9);
                else
                    display->drawFrame(offsets[i], 0, 24, 9);
            }

            YieldAudio();

            for (int i = 0; i < 4; i++)
            {
                voice.Gen->GetTab(i, tabval);
                if (strlen(tabval) == 0)
                    continue;
                    
                display->setDrawColor(i == activeTab ? 0 : 1);
                auto w = display->getStrWidth(tabval);
                display->setCursor(offsets[i] + 12 - w/2, 7);
                display->print(tabval);
            }

            YieldAudio();
        }

        inline void Render()
        {
            ActiveMenu->Render(GetDisplay());
            
            if (ActiveMenu->QuadMode && !ActiveMenu->DisableTabs)
                RenderHeader();

            if (millis() < overlayExpiration)
                overlayCallback(GetDisplay());
        }
        
        inline void Clear()
        {
            display.clearDisplay();
        }

        inline U8G2* GetDisplay()
        {
            return &display2;
        }

        // Method breaks down data transfer to the display into small chunks, and schedules transmissions
        // asynchronously
        inline void Transfer()
        {
            // don't call this function until the master has finished the previous transmission!
            // otherwise you'll block here
            while (!i2cMaster.finished()) {}

            int _page_start_offset = 2; // for the SH1106
            uint8_t DC_BYTE = 0x40;
            uint8_t pageCount = ((SCREEN_HEIGHT + 7) / 8);
            uint8_t bytesPerPage = SCREEN_WIDTH;
            //uint8_t* buffer = &testbuf[0];//display.getBufferPtr();
            uint8_t* buffer = display2.getBufferPtr();
            
            // page 0, byte 0; reset the ptr to start of buffer;
            if (p == 0 && pageBytesRemaining == bytesPerPage)
            {
                ptr = &buffer[0];
            }

            // New page boundary, issue start commands for that page
            if (sendStartBuffer)
            {
                txbufStart[0] = 0x00;
                txbufStart[1] = (uint8_t)(SH110X_SETPAGEADDR + p);
                txbufStart[2] = (uint8_t)(0x10 + ((0 + _page_start_offset) >> 4));
                txbufStart[3] = (uint8_t)((0 + _page_start_offset) & 0xF);
                i2cMaster.write_async(SCREEN_ADDRESS, txbufStart, 4, true);
                sendStartBuffer = false;
                return;
            }
            
            uint8_t to_write = min(pageBytesRemaining, (uint8_t)ChunkSize);

            txbuf[0] = DC_BYTE;
            for (int i = 0; i < to_write; i++)
                txbuf[i+1] = ptr[i];

            i2cMaster.write_async(SCREEN_ADDRESS, txbuf, to_write+1, true);

            ptr += to_write;
            pageBytesRemaining -= to_write;
            if (pageBytesRemaining == 0)
            {
                p = (p+1) % pageCount;
                pageBytesRemaining = bytesPerPage;
                sendStartBuffer = true;
            }
        }

    private:
        bool sendStartBuffer = true;
        int p = 0;
        uint8_t pageBytesRemaining = SCREEN_WIDTH;
        uint8_t *ptr;

        uint8_t txbufStart[4];
        uint8_t txbuf[ChunkSize+1];
    };

    extern DisplayManager displayManager;
}
