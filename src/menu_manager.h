#pragma once

//#include <Wire.h>
#include <i2c_driver_wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "logging.h"
#include "menu.h"

class MenuManager
{
    const int SCREEN_WIDTH = 128;
    const int SCREEN_HEIGHT = 64;
    const int OLED_RESET = -1;
    const int SCREEN_ADDRESS = 0x3C;

    Adafruit_SSD1306 display;

public:
    Menu* ActiveMenu;

    inline MenuManager() : display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET, 1000000, 1000000)
    {
        ActiveMenu = 0;
    }

    inline void Init()
    {
        if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) 
        {
            Serial.println(F("SSD1306 allocation failed"));
        }
    }

    inline void HandlePotUpdate(int idx, float value)
    {
        if (!ActiveMenu)
            return;

        int selectedIdx = (value-0.001) * ActiveMenu->Length;
        while(ActiveMenu->SelectedItem < selectedIdx)
            ActiveMenu->MoveDown();
        while(ActiveMenu->SelectedItem > selectedIdx)
            ActiveMenu->MoveUp();
    }

    inline void Render()
    {
        ActiveMenu->Render(GetDisplay());
    }

    inline void Transfer()
    {
        display.display();
    }

    inline void Clear()
    {
        display.clearDisplay();
    }

    inline Adafruit_SSD1306* GetDisplay()
    {
        return &display;
    }
};
