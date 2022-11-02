#pragma once

#include <functional>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "audio_yield.h"
#include <fonts/font.h>
#include <fonts/font2.h>
#include <fonts/font3.h>
#include "controls.h"

namespace Cyber
{
    class Menu;
    void HandleEncoderDefault(Menu* menu, int tick);
    void HandleEncoderSwitchDefault(Menu* menu, bool value);
    void HandlePotDefault(Menu* menu, int idx, float value);
    void HandleSwitchDefault(Menu* menu, int idx, bool value);

    class Menu
    {
        const int Height = 4;
        int Length; // effective length, can be shorter than maxLength (N)

    public:
        const int MaxLength = 32;
        const char* Captions[32] = {0};
        int16_t Values[32] = {0};
        int16_t Min[32] = {0};
        int16_t Max[32] = {0};
        int16_t Ticks[32] = {0};
        std::function<void(int, int16_t, char*)> Formatters[32];
        std::function<void(int, int16_t)> ValueChangedCallback = 0;
        std::function<void(Adafruit_SH1106G*)> RenderCustomDisplayCallback = 0;
        
        //Todo: Implement these, build "default" implementations that work for most menus, like switch handling which should almost always be the same
        std::function<void(Menu*, int)> HandleEncoderCallback = HandleEncoderDefault;
        std::function<void(Menu*, bool)> HandleEncoderSwitchCallback = HandleEncoderSwitchDefault;
        std::function<void(Menu*, int, float)> HandlePotCallback = HandlePotDefault;
        std::function<void(Menu*, int, bool)> HandleSwitchCallback = HandleSwitchDefault;

        int TopItem = 0;
        int SelectedItem = 0;
        bool EditMode = false; // scroll vs edit
        bool EnableSelection = true;
        bool QuadMode = false;
        bool CustomOnlyMode = false;

        inline Menu()
        {
            for (int i = 0; i < MaxLength; i++)
            {
                Formatters[i] = [](int idx, int16_t v, char* dest) { sprintf(dest, "%d", v); };
            }

            // set default tick and max size
            for (int i = 0; i < 32; i++)
            {
                Ticks[i] = 1;
                Max[i] = 100;
            }
        }

        inline void ReapplyAllValues()
        {
            // Reads all the values and re-sets them to the same value, invoking the callbacks along the way
            // useful for ensuring all values applied to other components that interact with callback

            for (int i = 0; i < Length; i++)
                SetValue(i, Values[i]);            
        }

        inline void HandleEncoder(int tick)
        {
            HandleEncoderCallback(this, tick);
        }

        inline void HandleEncoderSwitch(bool value)
        {
            HandleEncoderSwitchCallback(this, value);
        }

        inline void HandlePot(int idx, float value)
        {
            //LogInfof("Pot momentum: %.4f", controls.GetPotMomentum(0));
            //LogInfof("Pot excursion point: %.4f", controls.GetPotExcursionPoint(0));
            //LogInfof("Pot value: %.4f", controls.GetPot(0).Value*1023);
            HandlePotCallback(this, idx, value);
        }

        inline void HandleSwitch(int idx, bool value)
        {
            HandleSwitchCallback(this, idx, value);
        }

        inline void SetSelectedItem(int idx)
        {
            if (idx < 0) idx = 0;
            if (idx >= Length) idx = Length - 1;

            while(SelectedItem < idx)
                MoveDown();
            while(SelectedItem > idx)
                MoveUp();
        }

        inline void TickValue(int idx, int ticks)
        {
            if (idx >= Length || idx < 0)
                return;

            int delta = Ticks[idx] * ticks;
            int newValue = Values[idx] + delta;
            SetValue(idx, newValue);
        }

        inline void SetValue(int idx, int16_t value)
        {
            if (idx >= Length || idx < 0)
                return;

            if (value < Min[idx]) value = Min[idx];
            if (value > Max[idx]) value = Max[idx];
            Values[idx] = value;

            if (ValueChangedCallback != 0)
                ValueChangedCallback(idx, value);
        }

        inline void SetValueF(int idx, float value)
        {
            if (idx >= Length || idx < 0)
                return;

            if (value < 0) value = 0;
            if (value > 1) value = 1;
            int16_t v = Min[idx] + value * (Max[idx] - Min[idx]);
            Values[idx] = v;

            if (ValueChangedCallback != 0)
                ValueChangedCallback(idx, v);
        }

        inline int GetLength()
        {
            return Length;
        }

        inline void SetLength(int len)
        {
            if (len > MaxLength)
                len = MaxLength;

            Length = len;
        }

        inline int GetStringWidth(Adafruit_SH1106G* display, const char* str)
        {
            int16_t x, y;
            uint16_t w, h;
            display->getTextBounds(str, 0, 0, &x, &y, &w, &h);
            return w;
        }
        
        inline void Render(Adafruit_SH1106G* display)
        {
            if (!CustomOnlyMode)
            {
                if (QuadMode)
                    RenderQuad(display);
                else
                    RenderSerial(display);
            }

            if (RenderCustomDisplayCallback != 0)
                RenderCustomDisplayCallback(display);
        }

        inline void RenderSerial(Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setFont(&AtlantisInternational_jen08pt7b);
            display->setTextSize(1);
                
            char val[16];

            for (int i = 0; i < Height; i++)
            {
                YieldAudio();
                auto item = TopItem + i;
                if (item >= Length)
                    break;

                bool isSelected = (item == SelectedItem) && EnableSelection;

                if (isSelected)
                {
                    display->fillRect(0, 16 * i, display->width(), 16, SH110X_WHITE);
                    display->setTextColor(SH110X_BLACK);
                }
                else
                {
                    display->setTextColor(SH110X_WHITE);
                }

                YieldAudio();
                display->setCursor(2, 10 + 16 * i);
                display->println(Captions[item]);

                YieldAudio();
                Formatters[item](item, Values[item], val);
                int w = GetStringWidth(display, val);
                int x = display->width() - w - 2;
                int y = 10 + 16 * i;
                display->setCursor(x, y);
                if (isSelected && EditMode)
                    display->fillTriangle(x-8, y+1, x-8, y-7, x-4, y-3, SH110X_BLACK);
                display->println(val);
            }
        }

        inline void RenderQuad(Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setTextSize(1);

            char val[16];

            for (int k = 0; k < 4; k++)
            {
                YieldAudio();
                
                int item;
                if (k == 0) item = TopItem + 0;
                if (k == 1) item = TopItem + 3;
                if (k == 2) item = TopItem + 1;
                if (k == 3) item = TopItem + 2;
                
                if (item >= Length)
                    continue;

                bool isSelected = (item == SelectedItem) && EnableSelection;

                int x = k % 2;
                int y = k / 2;

                if (isSelected)
                {
                    display->fillRect(x * 64, 2 * 16 * y, display->width() / 2, 32, SH110X_WHITE);
                    display->setTextColor(SH110X_BLACK);
                }
                else
                {
                    display->setTextColor(SH110X_WHITE);
                }

                YieldAudio();

                int w = GetStringWidth(display, Captions[item]);
                int xPos = x == 0 ? 2 : display->width() - 2 - w;
                display->setCursor(xPos, 10 + 32 * y);
                display->println(Captions[item]);

                YieldAudio();
                Formatters[item](item, Values[item], val);
                w = GetStringWidth(display, val);
                xPos = x == 0 ? 2 : display->width() - 2 - w;
                display->setCursor(xPos, 10 + 16 * (2*y+1));
                display->println(val);
            }
        }

        inline void MoveDownPage()
        {
            MoveDown();
            MoveDown();
            MoveDown();
            MoveDown();
        }

        inline void MoveDown()
        {
            SelectedItem++;
            if (SelectedItem >= Length)
            {
                if (QuadMode)
                {
                    SelectedItem = Length - 1; // Can't roll over in quad mode
                }
                else
                {
                    SelectedItem = 0;
                    TopItem = 0;
                }
            }
            if (SelectedItem - TopItem >= Height)
            {
                if (QuadMode)
                {
                    TopItem += 4;
                    SelectedItem = TopItem;
                }
                else
                {
                    TopItem++;
                }
            }
        }

        inline void MoveUpPage()
        {
            MoveUp();
            MoveUp();
            MoveUp();
            MoveUp();
        }

        inline void MoveUp()
        {
            SelectedItem--;
            if (SelectedItem < 0)
            {
                if (QuadMode)
                {
                    SelectedItem = 0; // Can't roll over in quad mode
                    TopItem = 0;
                }
                else
                {
                    SelectedItem = Length - 1;
                    TopItem = SelectedItem - Height + 1;
                }
            }
            if (SelectedItem < TopItem)
            {
                if (QuadMode)
                {
                    TopItem -= 4;
                    if (TopItem < 0)
                        TopItem = 0;
                }
                else
                {
                    TopItem = SelectedItem;
                }
            }
        }
    };
}
