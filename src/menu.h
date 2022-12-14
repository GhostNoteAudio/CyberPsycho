#pragma once

#include <functional>
#include <U8g2lib.h>
#include "audio_yield.h"
#include "controls.h"
#include "fonts.h"

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

        int lastSetValueIdx = -1;
        int lastSetValueTime = 0;

    public:
        const int MaxLength = 32;
        const char* Captions[32] = {0};
        float Values[32] = {0};
        int16_t Min[32] = {0};
        uint16_t Steps[32] = {0};
        std::function<void(int, float, int, char*)> Formatters[32];
        std::function<void(int, float, int)> ValueChangedCallback = 0;
        std::function<void(U8G2*)> RenderCustomDisplayCallback = 0;
        
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
        bool DisableTabs = false;
        bool CustomOnlyMode = false;

        inline Menu()
        {
            for (int i = 0; i < MaxLength; i++)
            {
                Formatters[i] = [this](int idx, float v, int sv, char* dest) 
                {
                    Steps[idx] > 0 
                        ? sprintf(dest, "%d", sv) 
                        : sprintf(dest, "%.1f", v*100.0f); 
                };
            }
        }

        inline int GetScaledValue(int idx)
        {
            return Steps[idx] > 0 
                ? Min[idx] + (int)(Values[idx] * Steps[idx] * 1.000001f)
                : Min[idx] + Values[idx];
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

            float delta = 1.0f/Steps[idx] * ticks;
            float newValue = Values[idx] + delta;
            SetValue(idx, newValue);
        }

        inline void SetValue(int idx, float value)
        {
            if (value < 0) value = 0;
            if (value > 1) value = 1;

            if (idx >= Length || idx < 0)
                return;

            Values[idx] = value;
            lastSetValueIdx = idx;
            lastSetValueTime = micros();

            if (ValueChangedCallback != 0)
                ValueChangedCallback(idx, value, GetScaledValue(idx));
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

        inline int GetStringWidth(U8G2* display, const char* str)
        {
            return display->getStrWidth(str);
        }
        
        inline void Render(U8G2* display)
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

        inline void RenderSerial(U8G2* display)
        {
            display->clearDisplay();
            display->setFont(DEFAULT_FONT); 
                
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
                    display->setDrawColor(1);
                    display->drawBox(0, 16 * i, display->getWidth(), 16);
                    display->setDrawColor(0);
                }
                else
                {
                    display->setDrawColor(1);
                }

                YieldAudio();
                display->setCursor(2, 11 + 16 * i);
                display->print(Captions[item]);

                YieldAudio();
                Formatters[item](item, Values[item], GetScaledValue(item), val);
                int w = GetStringWidth(display, val);
                int x = display->getWidth() - w - 2;
                int y = 11 + 16 * i;
                display->setCursor(x, y);
                if (isSelected && EditMode)
                {
                    display->setDrawColor(0);
                    display->drawTriangle(x-8, y+1, x-8, y-7, x-4, y-3);
                }
                display->print(val);
            }
        }

        inline void RenderQuad(U8G2* display)
        {
            display->clearDisplay();
            display->setFont(DEFAULT_FONT);

            char val[16];

            display->setDrawColor(1);
            display->drawFrame(0, 31, 63, 17);
            display->drawFrame(65, 31, 63, 17);
            display->drawFrame(0, 47, 63, 17);
            display->drawFrame(65, 47, 63, 17);

            bool setValueVisible = false;
            int pageCount = (Length+3) / 4;
            int pageOffset = 64 - (pageCount-1) * 3;
            int currentPage = TopItem / 4;
            for (int i = 0; i < pageCount; i++)
            {
                if (i == currentPage)
                    display->drawFrame(pageOffset + i * 6-1, 4-1, 3, 3);
                else
                    display->drawPixel(pageOffset + i * 6, 4);
            }
            
            for (int k = 0; k < 4; k++)
            {
                YieldAudio();
                
                int item = 999;
                if (k == 0) item = TopItem + 0;
                if (k == 1) item = TopItem + 3;
                if (k == 2) item = TopItem + 1;
                if (k == 3) item = TopItem + 2;
                
                if (item >= Length)
                    continue;

                setValueVisible |= item == lastSetValueIdx;

                int x = k % 2;
                int y = k / 2;
                YieldAudio();

                int w = GetStringWidth(display, Captions[item]);
                int xPos = x == 0 ? 32 - w/2 : 96 - w/2;
                display->setCursor(xPos, y == 0 ? 44 : 60);
                display->print(Captions[item]);

                YieldAudio();
                display->drawBox(x == 0 ? 1 : 66, y == 0 ? 32 : 48, Values[item] * 61, 2);
            }

            if (setValueVisible && (micros() - lastSetValueTime) < 1000000)
            {
                int item = lastSetValueIdx;
                Formatters[item](item, Values[item], GetScaledValue(item), val);
                int w = GetStringWidth(display, val);
                display->setCursor(64 - w/2, 24);
                display->print(val);
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
