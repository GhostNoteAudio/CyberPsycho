#include "menus.h"
#include <fonts/font.h>
#include <fonts/font2.h>
#include <fonts/font3.h>

namespace Cyber
{
    namespace Menus
    {
        Menu initMenu;
        Menu globalMenu;

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
        }
    }
}
