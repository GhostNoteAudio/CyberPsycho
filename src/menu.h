#pragma once

#include <functional>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "audio_yield.h"

namespace Cyber
{
    class Menu
    {
        const int Height = 4;
        int Length; // effective length, can be shorter than maxLength (N)

    public:
        const int MaxLength = 32;
        const char* Captions[32] = {0};
        float Values[32] = {0};
        std::function<void(float, char*)> Formatters[32];
        std::function<void(int, float)> ValueChangedCallback = 0;

        int TopItem;
        int SelectedItem;
        bool EnableSelection;
        bool QuadMode;

        inline Menu()
        {
            for (int i = 0; i < MaxLength; i++)
            {
                Formatters[i] = [](float v, char* dest) { sprintf(dest, "%.2f", v); };
            }
        }

        inline void SetValue(int idx, float value)
        {
            if (idx >= Length)
                return;

            Values[idx] = value;

            if (ValueChangedCallback != 0)
                ValueChangedCallback(idx, value);
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

        inline int GetStringWidth(Adafruit_SSD1306* display, const char* str)
        {
            int16_t x, y;
            uint16_t w, h;
            display->getTextBounds(str, 0, 0, &x, &y, &w, &h);
            return w;
        }
        
        inline void Render(Adafruit_SSD1306* display)
        {
            if (QuadMode)
                RenderQuad(display);
            else
                RenderSerial(display);
        }


        inline void RenderSerial(Adafruit_SSD1306* display)
        {
            display->clearDisplay();
            display->setTextSize(1);

            char val[16];

            for (int i = 0; i < Height; i++)
            {
                YieldAudio();
                auto item = TopItem + i;
                bool isSelected = (item == SelectedItem) && EnableSelection;

                if (isSelected)
                {
                    display->fillRect(0, 16 * i, display->width(), 16, SSD1306_WHITE);
                    display->setTextColor(SSD1306_BLACK);
                }
                else
                {
                    display->setTextColor(SSD1306_WHITE);
                }

                display->setCursor(2, 4 + 16 * i);
                display->println(Captions[item]);

                Formatters[item](Values[item], val);
                
                int w = GetStringWidth(display, val);
                display->setCursor(display->width() - w - 2, 4 + 16 * i);
                display->println(val);
            }
        }

        inline void RenderQuad(Adafruit_SSD1306* display)
        {
            display->clearDisplay();
            display->setTextSize(1);

            char val[16];

            for (int i = 0; i < Height; i++)
            {
                YieldAudio();
                auto item = TopItem + i;
                if (item >= Length)
                    break;

                bool isSelected = (item == SelectedItem) && EnableSelection;

                int x = i % 2;
                int y = i / 2;

                if (isSelected)
                {
                    display->fillRect(x * 64, 2 * 16 * y, display->width() / 2, 32, SSD1306_WHITE);
                    display->setTextColor(SSD1306_BLACK);
                }
                else
                {
                    display->setTextColor(SSD1306_WHITE);
                }

                int w = GetStringWidth(display, Captions[item]);
                int xPos = x == 0 ? 2 : display->width() - 2 - w;
                display->setCursor(xPos, 4 + 32 * y);
                display->println(Captions[item]);

                Formatters[item](Values[item], val);
                
                w = GetStringWidth(display, val);
                xPos = x == 0 ? 2 : display->width() - 2 - w;
                display->setCursor(xPos, 4 + 16 * (2*y+1));
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