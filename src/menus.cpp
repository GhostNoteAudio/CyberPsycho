#include "menus.h"
#include <fonts/font.h>
#include <fonts/font2.h>
#include <fonts/font3.h>
#include "utils.h"
#include "logging.h"

namespace Cyber
{
    namespace Scope
    {
        extern uint16_t data[128];
        extern uint8_t downsampling;
        extern uint16_t triggerFreq;
    }

    namespace Menus
    {
        Menu* ActiveMenu;

        Menu initMenu;
        Menu globalMenu;
        Menu scopeMenu;

        void BuildScopeMenu()
        {
            scopeMenu.SetLength(4);
            scopeMenu.CustomOnlyMode = true;
            scopeMenu.RenderCustomDisplayCallback = [](Adafruit_SSD1306* display)
            {
                display->clearDisplay();
                float h = display->height();
                
                for (int x = 0; x < display->width(); x++)
                {
                    uint8_t y = Scope::data[x] >> 6; // 12 bit to 6 bit (0-63)
                    display->drawPixel(x, (h-1) - y, SSD1306_WHITE);
                }

                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(1);
                display->setTextColor(SSD1306_BLACK);
                display->setCursor(1, 62);
                char readout[16];
                auto readoutVal = scopeMenu.Values[3];
                if (readoutVal < 0.25)
                {}
                else if (readoutVal < 0.5)
                {
                    display->fillRect(0, 55, 60, 9, SSD1306_WHITE);
                    sprintf(readout, "Min: %d", Utils::Min(Scope::data, 128));
                    display->println(readout);
                }
                else if (readoutVal < 0.75)
                {
                    display->fillRect(0, 55, 60, 9, SSD1306_WHITE);
                    sprintf(readout, "Max: %d", Utils::Max(Scope::data, 128));
                    display->println(readout);
                }
                else
                {
                    display->fillRect(0, 55, 60, 9, SSD1306_WHITE);
                    sprintf(readout, "Mean: %d", (int)Utils::Mean(Scope::data, 128));
                    display->println(readout);
                }
            };
            scopeMenu.HandlePotCallback = [](Menu* menu, int idx, float value)
            {
                if (idx == 1)
                    Scope::downsampling = (int)(value * 7.99);
                if (idx == 2)
                    Scope::triggerFreq = 1024 + (int)(value * 4096);
                if (idx == 3)
                {
                    menu->SetValue(idx, value);
                }
            };
        }

        void BuildInitMenu()
        {
            initMenu.CustomOnlyMode = true;
            initMenu.RenderCustomDisplayCallback = [](Adafruit_SSD1306* display)
            {
                display->clearDisplay();
                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(2);
                display->setTextColor(SSD1306_WHITE);
                int w = initMenu.GetStringWidth(display, "Cyberpsycho");
                int offset = (display->width() - w) / 2;
                display->setCursor(offset, 22);
                display->println("Cyberpsycho");

                display->setTextSize(1);
                w = initMenu.GetStringWidth(display, "Ghost Note Audio");
                offset = (display->width() - w) / 2;
                display->setCursor(offset, 38);
                display->println("Ghost Note Audio");

                w = initMenu.GetStringWidth(display, VERSION);
                offset = (display->width() - w) / 2;
                display->setCursor(offset - 3, 49);
                display->println("v");
                display->setCursor(offset + 3, 49);
                display->println(VERSION);
            };
        }

        void BuildGlobalMenu()
        {
            globalMenu.Captions[0] = "CV 1 Offset";
            globalMenu.Captions[1] = "CV 2 Offset";
            globalMenu.Captions[2] = "CV 3 Offset";
            globalMenu.Captions[3] = "CV 4 Offset";

            globalMenu.Captions[4] = "Mod 1 Offset";
            globalMenu.Captions[5] = "Mod 2 Offset";
            globalMenu.Captions[6] = "Mod 3 Offset";
            globalMenu.Captions[7] = "Mod 4 Offset";

            globalMenu.Values[0] = 0.0;
            globalMenu.Values[1] = 0.0;
            globalMenu.Values[2] = 0.0;
            globalMenu.Values[3] = 0.0;
            globalMenu.Values[4] = 0.5;
            globalMenu.Values[5] = 0.5;
            globalMenu.Values[6] = 0.5;
            globalMenu.Values[7] = 0.5;

            for (int i = 0; i < 8; i++)
            {
                globalMenu.Formatters[i] = [](float v, char* s) { sprintf(s, "%.3f", (2*v-1)); };
            }
            
            globalMenu.SetLength(8);
            globalMenu.SelectedItem = 0;
            globalMenu.TopItem = 0;
            globalMenu.EnableSelection = true;
            globalMenu.QuadMode = false;
        }

        void Init()
        {
            BuildInitMenu();
            BuildGlobalMenu();
            BuildScopeMenu();
        }

        void HandleEncoderDefault(Menu* menu, int tick)
        {
            if (tick == 1 && !menu->QuadMode)
                menu->MoveDown();
            if (tick == 1 && menu->QuadMode)
                menu->MoveDownPage();
            if (tick == 1 && !menu->QuadMode)
                menu->MoveUp();
            if (tick == 1 && menu->QuadMode)
                menu->MoveUpPage();
        }

        void HandlePotDefault(Menu* menu, int idx, float value)
        {
            if (menu->QuadMode)
            {
                menu->SetValue(menu->TopItem + idx, value);
            }
            else if (!menu->QuadMode && idx == 0)
            {
                menu->SetValue(menu->SelectedItem, value);
            }

            // hack for now
            if (idx == 1)
            {
                int it = (value * 0.999) * menu->GetLength();
                menu->SetSelectedItem(it);
            }
        }

        void HandleSwitchDefault(Menu* menu, int idx, bool value)
        {
            if (idx == 0 && value)
                ActiveMenu = &globalMenu;
            if (idx == 1 && value)
                ActiveMenu = &scopeMenu;
        }
    }
}
