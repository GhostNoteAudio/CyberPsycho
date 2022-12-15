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

    const int MENU_MAX_SIZE = 64;

    class Menu
    {
        const int Height = 4;
        int Length; // effective length, can be shorter than MENU_MAX_SIZE
        uint64_t SectionBreaks = 0;
        int lastSetValueIdx = -1;
        int lastSetValueTime = 0;

    public:
        const char* Captions[MENU_MAX_SIZE] = {0};
        float Values[MENU_MAX_SIZE] = {0};
        int16_t Min[MENU_MAX_SIZE] = {0};
        uint16_t Steps[MENU_MAX_SIZE] = {0};
        std::function<void(int, float, int, char*)> Formatters[MENU_MAX_SIZE];
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
        bool QuadMode = false;
        bool DisableTabs = false;
        bool CustomOnlyMode = false;

        inline Menu()
        {
            for (int i = 0; i < MENU_MAX_SIZE; i++)
            {
                Formatters[i] = [this](int idx, float v, int sv, char* dest) 
                {
                    Steps[idx] > 0 
                        ? sprintf(dest, "%d", sv) 
                        : sprintf(dest, "%.1f", v*100.0f); 
                };
            }
        }

        inline void AddSectionBreak(int idxAfter)
        {
            SectionBreaks |= ((uint64_t)1) << idxAfter;
        }

        inline void RemoveSectionBreak(int idxAfter)
        {
            SectionBreaks &= ~(((uint64_t)1) << idxAfter);
        }

        inline bool IsSectionBreak(int idxAfter)
        {
            auto v = SectionBreaks & (((uint64_t)1) << idxAfter);
            return v > 0;
        }

        inline int GetOffsetInSection(int idx = -1)
        {
            idx = idx == -1 ? TopItem : idx;
            int count = 0;
            while(true)
            {
                if (idx == 0)
                    break;
                if (IsSectionBreak(idx-1))
                    break;

                count++;
                idx--;
            }

            return count;
        }

        inline int GetTotalInSection(int idx = -1)
        {
            idx = idx == -1 ? TopItem : idx;
            int count = GetOffsetInSection(idx);

            while(true)
            {
                if (idx == Length-1)
                    break;
                if (IsSectionBreak(idx))
                    break;

                count++;
                idx++;
            }

            return count + 1;
        }

        inline bool IsVisible(int idx)
        {
            if (idx < TopItem) return false;
            if (idx == TopItem) return true;
            if (idx == TopItem + 1) return !IsSectionBreak(TopItem);
            if (idx == TopItem + 2) return !IsSectionBreak(TopItem) && !IsSectionBreak(TopItem + 1);
            if (idx == TopItem + 3) return !IsSectionBreak(TopItem) && !IsSectionBreak(TopItem + 1) && !IsSectionBreak(TopItem + 2);
            return false;
        }

        inline int GetPage(int idx = -1)
        {
            return GetOffsetInSection(idx) / 4;
        }

        inline int GetPageCount(int idx = -1)
        {
            return (GetTotalInSection(idx)+3) / 4;
        }

        inline int GetScaledValue(int idx)
        {
            if (Steps[idx] == 0)
                return Min[idx] + Values[idx];

            float v = Values[idx];
            if (v >= 1)
                v = 0.999999f;

            return Min[idx] + (int)(v * Steps[idx]);
        }

        inline void SetScaledValue(int idx, int scaledValue)
        {
            scaledValue -= Min[idx];
            float halfstep = 0.5f / Steps[idx];
            float v = ((float)scaledValue) / Steps[idx] + halfstep;
            SetValue(idx, v);
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

            if (!QuadMode)
            {
                while(SelectedItem < idx)
                    MoveDown();
                while(SelectedItem > idx)
                    MoveUp();
            }
            else
            {
                TopItem = idx;
                SelectedItem = TopItem;
            }
        }

        inline void TickValue(int idx, int ticks)
        {
            if (idx >= Length || idx < 0)
                return;

            if (Steps[idx] == 0)
            {
                // if no steps, change by 1%
                SetValue(idx, Values[idx] + 0.01 * ticks);
            }
            else
            {
                int sv = GetScaledValue(idx);
                int newSv = sv + ticks;
                SetScaledValue(idx, newSv);
            }
        }

        inline void SetValue(int idx, float value)
        {
            LogInfof("Setting %d to %.3f", idx, value);

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
            if (len > MENU_MAX_SIZE)
                len = MENU_MAX_SIZE;

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

                bool isSelected = (item == SelectedItem);

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
            int pageCount = GetPageCount();
            int currentPage = GetPage();

            int pageOffsetPx = 64 - (pageCount-1) * 3;
            if (!DisableTabs)
            {
                for (int i = 0; i < pageCount; i++)
                {
                    if (i == currentPage)
                        display->drawFrame(pageOffsetPx + i * 6-1, 4-1, 3, 3);
                    else
                        display->drawPixel(pageOffsetPx + i * 6, 4);
                }
            }
            
            for (int p = 0; p < 4; p++)
            {
                YieldAudio();
                
                int item = TopItem + p;
                if (item >= Length)
                    continue;

                setValueVisible |= item == lastSetValueIdx;

                int x = (p == 2 || p == 3);
                int y = (p == 1 || p == 2);

                int w = GetStringWidth(display, Captions[item]);
                int xPos = x == 0 ? 32 - w/2 : 96 - w/2;
                display->setCursor(xPos, y == 0 ? 44 : 60);
                display->print(Captions[item]);

                YieldAudio();
                display->drawBox(x == 0 ? 1 : 66, y == 0 ? 32 : 48, Values[item] * 61, 2);

                if (IsSectionBreak(item))
                    break;
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

        inline void MoveDown()
        {
            if (QuadMode)
            {
                if (TopItem + 4 >= Length)
                    return;
                
                bool crossesBreak = IsSectionBreak(TopItem) 
                                 || IsSectionBreak(TopItem+1) 
                                 || IsSectionBreak(TopItem+2) 
                                 || IsSectionBreak(TopItem+3);
                
                if (crossesBreak)
                    return;
                
                TopItem += 4;
                SelectedItem = TopItem;
                return;
            }

            SelectedItem++;
            if (SelectedItem >= Length)
            {
                SelectedItem = 0;
                TopItem = 0;
            }
            if (SelectedItem - TopItem >= Height)
            {
                TopItem++;
            }
        }

        inline void MoveUp()
        {
            if (QuadMode)
            {
                if (TopItem - 4 < 0)
                    return;
                
                bool crossesBreak = IsSectionBreak(TopItem-1) 
                                 || IsSectionBreak(TopItem-2) 
                                 || IsSectionBreak(TopItem-3) 
                                 || IsSectionBreak(TopItem-4);
                
                if (crossesBreak)
                    return;
                
                TopItem -= 4;
                SelectedItem = TopItem;
                return;
            }

            SelectedItem--;
            if (SelectedItem < 0)
            {
                SelectedItem = Length - 1;
                TopItem = SelectedItem - Height + 1;
            }
            if (SelectedItem < TopItem)
            {
                TopItem = SelectedItem;
            }
        }
    };
}
