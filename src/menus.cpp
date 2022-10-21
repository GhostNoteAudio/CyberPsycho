#include "menus.h"
#include <fonts/font.h>
#include <fonts/font2.h>
#include <fonts/font3.h>
#include "utils.h"
#include "logging.h"
#include "input_processor.h"
#include "scope.h"

namespace Cyber
{   
    namespace Menus
    {
        Menu* ActiveMenu;

        Menu initMenu;
        Menu globalMenu;
        Menu scopeMenu;
        Menu calibrateMenu;
        Menu voiceMenu;
        Menu pitchTrigMenu;

        void BuildScopeMenu()
        {
            scopeMenu.SetLength(4);
            scopeMenu.CustomOnlyMode = true;
            scopeMenu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
            {
                display->clearDisplay();
                float h = display->height();
                
                for (int x = 0; x < display->width(); x++)
                {
                    uint8_t y = Scope::data[x] >> 6; // 12 bit to 6 bit (0-63)
                    display->drawPixel(x, (h-1) - y, SH110X_WHITE);
                }

                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(1);
                display->setTextColor(SH110X_BLACK);
                display->setCursor(1, 62);
                char readout[16];
                auto readoutVal = scopeMenu.Values[3];
                if (readoutVal < 0.25)
                {}
                else if (readoutVal < 0.5)
                {
                    display->fillRect(0, 55, 60, 9, SH110X_WHITE);
                    sprintf(readout, "Min: %d", Utils::Min(Scope::data, 128));
                    display->println(readout);
                }
                else if (readoutVal < 0.75)
                {
                    display->fillRect(0, 55, 60, 9, SH110X_WHITE);
                    sprintf(readout, "Max: %d", Utils::Max(Scope::data, 128));
                    display->println(readout);
                }
                else
                {
                    display->fillRect(0, 55, 60, 9, SH110X_WHITE);
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
            initMenu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
            {
                display->clearDisplay();
                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(2);
                display->setTextColor(SH110X_WHITE);
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
            globalMenu.Captions[0] = "Clock Source";
            globalMenu.Captions[1] = "Clock Scale";
            globalMenu.Captions[2] = "BPM";
            globalMenu.Captions[3] = "Gate Filter";
            globalMenu.Captions[4] = "> Load Preset";
            globalMenu.Captions[5] = "> Save Preset";
            globalMenu.Captions[6] = "> Init Program";
            globalMenu.Captions[7] = "> Calibrate";
            globalMenu.Captions[8] = "> Scope";

            globalMenu.Values[0] = 0.0;
            globalMenu.Values[1] = 0.0;
            globalMenu.Values[2] = 0.358; // 120 bpm
            globalMenu.Values[3] = 0.0;
            globalMenu.Values[4] = 0.0;
            globalMenu.Values[5] = 0.0;
            globalMenu.Values[6] = 0.0;
            globalMenu.Values[7] = 0.0;
            globalMenu.Values[8] = 0.0;

            int clockScaleLut[12] = {1,2,4,8,12,16,24,32,48,64,96,128};
            const char* clockSources[3] = {"Int", "Ext", "Midi"};
            const char* gateFilters[4] = {"Off", "Mild", "Normal", "High"};
            int gateFilterValues[4] = {255, 128, 64, 16};

            globalMenu.Ticks[0] = 1.0/(sizeof(clockSources) / sizeof(char*))+0.001;
            globalMenu.Ticks[1] = 1.0/(sizeof(clockScaleLut) / sizeof(int))+0.001;
            globalMenu.Ticks[2] = 1.0/280.0;
            globalMenu.Ticks[3] = 1.0/(sizeof(gateFilters) / sizeof(char*))+0.001;

            globalMenu.Formatters[0] = [clockSources](int idx, float v, char* s) { strcpy(s, clockSources[(int)(v*2.999)]); };
            globalMenu.Formatters[1] = [clockScaleLut](int idx, float v, char* s) { sprintf(s, "%d", clockScaleLut[(int)(v*12.999)]); };
            globalMenu.Formatters[2] = [](int idx, float v, char* s) { sprintf(s, "%d", (int)(20 + 280*v)); };
            globalMenu.Formatters[3] = [gateFilters](int idx, float v, char* s) { strcpy(s, gateFilters[(int)(v*3.999)]); };
            for (int i=4; i<=8; i++)
                globalMenu.Formatters[i] = [](int idx, float v, char* s) { strcpy(s, ""); };

            globalMenu.SetLength(9);
            globalMenu.SelectedItem = 0;
            globalMenu.TopItem = 0;
            globalMenu.EnableSelection = true;
            globalMenu.QuadMode = false;

            globalMenu.ValueChangedCallback = [gateFilterValues](int idx, float value)
            {
                if (idx == 3)
                    inProcessor.GateSpeed = gateFilterValues[(int)(value*3.999)];
            };

            globalMenu.HandleSwitchCallback = [](Menu* menu, int idx, bool value)
            {
                if (idx == 3 && value)
                {
                    if (menu->SelectedItem == 7)
                        ActiveMenu = &calibrateMenu;
                    else if (menu->SelectedItem == 8)
                        ActiveMenu = &scopeMenu;
                }
                else
                    HandleSwitchDefault(menu, idx, value);
            };
        }

        void BuildCalibrateMenu()
        {
            calibrateMenu.Captions[0] = "CV1 Offset";
            calibrateMenu.Captions[1] = "CV2 Offset";
            calibrateMenu.Captions[2] = "CV3 Offset";
            calibrateMenu.Captions[3] = "CV4 Offset";
            calibrateMenu.Captions[4] = "Mod1 Offset";
            calibrateMenu.Captions[5] = "Mod2 Offset";
            calibrateMenu.Captions[6] = "Mod3 Offset";
            calibrateMenu.Captions[7] = "Mod4 Offset";
            calibrateMenu.Captions[8] = "CV1 Scale";
            calibrateMenu.Captions[9] = "CV2 Scale";
            calibrateMenu.Captions[10] = "CV3 Scale";
            calibrateMenu.Captions[11] = "CV4 Scale";
            calibrateMenu.Captions[12] = "Mod1 Scale";
            calibrateMenu.Captions[13] = "Mod2 Scale";
            calibrateMenu.Captions[14] = "Mod3 Scale";
            calibrateMenu.Captions[15] = "Mod4 Scale";

            for (int i = 0; i < 8; i++)
                calibrateMenu.Formatters[i] = [](int idx, float v, char* s) { sprintf(s, "%d", (int)((2*v-1)*256)); };

            for (int i = 8; i < 16; i++)
                calibrateMenu.Formatters[i] = [](int idx, float v, char* s) { sprintf(s, "%.3f", 0.8+v*0.4); };

            calibrateMenu.ValueChangedCallback = [](int idx, float v)
            {
                if (idx < 4) inProcessor.OffsetCv[idx] = (int)((2*v-1)*256);
                else if (idx < 8) inProcessor.OffsetMod[idx-4] = (int)((2*v-1)*256);
                else if (idx < 12) inProcessor.ScaleCv[idx-8] = 0.8+v*0.4;
                else if (idx < 16) inProcessor.ScaleMod[idx-12] = 0.8+v*0.4;
            };

            calibrateMenu.SetLength(16);
            calibrateMenu.SelectedItem = 0;
            calibrateMenu.TopItem = 0;
            calibrateMenu.EnableSelection = true;
            calibrateMenu.QuadMode = false;
        }

        void BuildVoiceMenu()
        {
            voiceMenu.Captions[0] = "In Gain";
            voiceMenu.Captions[1] = "Out Gain";
            //voiceMenu.Captions[x] = "> Load Preset";
            //voiceMenu.Captions[x] = "> Save Preset";
            voiceMenu.Captions[2] = "> Init Voice";
            voiceMenu.Captions[3] = "> Clear All Mods";
            voiceMenu.Captions[4] = "Midi Ch";
            voiceMenu.Captions[5] = "Audio In L";
            voiceMenu.Captions[6] = "Audio In R";
            voiceMenu.Captions[7] = "Audio Out L";
            voiceMenu.Captions[8] = "Audio Out R";
            voiceMenu.Captions[9] = "CV In";
            voiceMenu.Captions[10] = "Gate In";

            voiceMenu.Formatters[0] = [](int idx, float v, char* s) { sprintf(s, "%.2fdB", -12+24*v); };
            voiceMenu.Formatters[1] = [](int idx, float v, char* s) { sprintf(s, "%.2fdB", -12+24*v); };
            voiceMenu.Formatters[2] = [](int idx, float v, char* s) { strcpy(s, ""); };
            voiceMenu.Formatters[3] = [](int idx, float v, char* s) { strcpy(s, ""); };
            voiceMenu.Formatters[4] = [](int idx, float v, char* s) 
            { 
                int ch = (int)(17.999*v);
                if (ch == 0) strcpy(s, "Off");
                else if (ch == 1) strcpy(s, "Omni");
                else sprintf(s, "%d", ch-1); 
            };
            
            for(int i=5; i<=10; i++)
                voiceMenu.Formatters[i] = [](int idx, float v, char* s) { sprintf(s, "%d", (int)(1+3.999*v)); };

            voiceMenu.SetLength(11);
            voiceMenu.SelectedItem = 0;
            voiceMenu.TopItem = 0;
            voiceMenu.EnableSelection = true;
            voiceMenu.QuadMode = false;
        }

        void BuildPitchTrigMenu()
        {
            pitchTrigMenu.SetLength(0);
            pitchTrigMenu.CustomOnlyMode = true;
            pitchTrigMenu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
            {
                display->clearDisplay();
                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(1);
                display->setTextColor(SH110X_WHITE);
                display->setCursor(30, 30);
                display->println("Coming Soon");
            };
        }

        void Init()
        {
            BuildInitMenu();
            BuildGlobalMenu();
            BuildScopeMenu();
            BuildCalibrateMenu();
            BuildVoiceMenu();
            BuildPitchTrigMenu();
        }

        void HandleEncoderDefault(Menu* menu, int tick)
        {
            if (!menu->EditMode)
            {
                if (tick == 1 && !menu->QuadMode)
                    menu->MoveDown();
                if (tick == 1 && menu->QuadMode)
                    menu->MoveDownPage();
                if (tick == -1 && !menu->QuadMode)
                    menu->MoveUp();
                if (tick == -1 && menu->QuadMode)
                    menu->MoveUpPage();
            }
            else if (menu->EditMode && !menu->QuadMode)
            {
                menu->TickValue(menu->SelectedItem, tick);
            }
        }

        void HandleEncoderSwitchDefault(Menu* menu, bool value)
        {
            if (value && !menu->QuadMode)
                menu->EditMode = !menu->EditMode;
        }

        void HandlePotDefault(Menu* menu, int idx, float value)
        {
            if (menu->QuadMode)
            {
                menu->SetValue(menu->TopItem + idx, value);
            }
        }

        void HandleSwitchDefault(Menu* menu, int idx, bool value)
        {
            LogInfof("Handling switch %d, value %d", idx, (int)value);
            
            if (idx == 0)
                modalState.Shift = value;
            else if (idx == 1 && value && modalState.Shift)
                ActiveMenu = &globalMenu;
            else if (idx == 2 && value && modalState.Shift)
                ActiveMenu = &voiceMenu;
            else if (idx == 3 && value && modalState.Shift)
                ActiveMenu = &pitchTrigMenu;

            if ((idx == 1 || idx == 2 || idx == 3) && modalState.Shift)
                ActiveMenu->EditMode = false;
        }
    }
}
