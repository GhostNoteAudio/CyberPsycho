#include "menus.h"
#include <fonts/font.h>
#include <fonts/font2.h>
#include <fonts/font3.h>
#include "utils.h"
#include "logging.h"
#include "input_processor.h"
#include "scope.h"
#include "generators/kick1.h"
#include "voices.h"
#include "generatorRegistry.h"

namespace Cyber
{
    extern Kick1 kick;

    namespace Menus
    {
        int lastChangeMillis = 0;
        int lastChangeIdx = 0;

        Menu* ActiveMenu;

        Menu initMenu;
        Menu globalMenu;
        Menu scopeMenu;
        Menu calibrateMenu;
        Menu voiceMenu;
        Menu pitchTrigMenu;
        Menu generatorSelectMenu;

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
                    if (x % 16 == 0)
                        YieldAudio();
                    uint8_t y = Scope::data[x] >> 6; // 12 bit to 6 bit (0-63)
                    display->drawPixel(x, (h-1) - y, SH110X_WHITE);
                }

                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(1);
                display->setTextColor(SH110X_BLACK);
                display->setCursor(65, 62);
                char readout[16];
                auto readoutVal = scopeMenu.Values[3];
                YieldAudio();

                if (readoutVal == 1)
                {
                    display->fillRect(64, 55, 60, 9, SH110X_WHITE);
                    sprintf(readout, "Min: %d", Utils::Min(Scope::data, 128));
                    display->println(readout);
                }
                else if (readoutVal == 2)
                {
                    display->fillRect(64, 55, 60, 9, SH110X_WHITE);
                    sprintf(readout, "Max: %d", Utils::Max(Scope::data, 128));
                    display->println(readout);
                }
                else if (readoutVal == 3)
                {
                    display->fillRect(64, 55, 60, 9, SH110X_WHITE);
                    sprintf(readout, "Mean: %d", (int)Utils::Mean(Scope::data, 128));
                    display->println(readout);
                }
                YieldAudio();

                bool shouldDisplayChange = (millis() - lastChangeMillis) < 2000;
                if (shouldDisplayChange)
                {
                    display->fillRect(0, 55, 60, 9, SH110X_WHITE);
                    display->setCursor(1, 62);
                    char readout[16];
                    int dsEffective = (1<<Scope::downsampling);

                    if (lastChangeIdx == 0) sprintf(readout, "Channel: %d", Scope::channel);
                    if (lastChangeIdx == 1) sprintf(readout, "Divide: %d", dsEffective);
                    if (lastChangeIdx == 2) sprintf(readout, "Freq: %d", Scope::triggerFreq);
                    display->println(readout);
                }
            };
            scopeMenu.HandlePotCallback = [](Menu* menu, int idx, float value)
            {
                if (idx == 0)
                    Scope::channel = (int)(value * 7.99);
                if (idx == 1)
                    Scope::downsampling = (int)(value * 9.99);
                if (idx == 2)
                    Scope::triggerFreq = 1024 + (int)(value * 4096);
                if (idx == 3)
                    menu->SetValue(idx, (int)(value*3.999));

                if (idx < 3)
                {
                    lastChangeIdx = idx;
                    lastChangeMillis = millis();
                }
            };
        }

        void BuildInitMenu()
        {
            initMenu.CustomOnlyMode = true;
            initMenu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
            {
                display->clearDisplay();
                YieldAudio();
                display->setFont(&AtlantisInternational_jen08pt7b);
                display->setTextSize(2);
                display->setTextColor(SH110X_WHITE);
                int w = initMenu.GetStringWidth(display, "Cyberpsycho");
                int offset = (display->width() - w) / 2;
                display->setCursor(offset, 22);
                display->print("Cyberpsycho");
                YieldAudio();
                
                display->setTextSize(1);
                w = initMenu.GetStringWidth(display, "Ghost Note Audio");
                offset = (display->width() - w) / 2;
                display->setCursor(offset, 38);
                display->print("Ghost Note Audio");
                YieldAudio();

                w = initMenu.GetStringWidth(display, VERSION);
                offset = (display->width() - w) / 2;
                display->setCursor(offset - 3, 49);
                display->print("v");
                display->setCursor(offset + 3, 49);
                display->print(VERSION);
                YieldAudio();
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

            globalMenu.Values[0] = 0;
            globalMenu.Values[1] = 0;
            globalMenu.Values[2] = 120;
            globalMenu.Values[3] = 0;
            globalMenu.Values[4] = 0;
            globalMenu.Values[5] = 0;
            globalMenu.Values[6] = 0;
            globalMenu.Values[7] = 0;
            globalMenu.Values[8] = 0;

            int clockScaleLut[12] = {1,2,4,8,12,16,24,32,48,64,96,128};
            const char* clockSources[3] = {"Int", "Ext", "Midi"};
            const char* gateFilters[4] = {"Off", "Mild", "Normal", "High"};
            int gateFilterValues[4] = {255, 128, 64, 16};

            globalMenu.Max[0] = 2;
            globalMenu.Max[1] = 11;
            globalMenu.Max[2] = 300;
            globalMenu.Max[3] = 3;

            globalMenu.Min[2] = 20;

            globalMenu.Formatters[0] = [clockSources](int idx, int16_t v, char* s) { strcpy(s, clockSources[v]); };
            globalMenu.Formatters[1] = [clockScaleLut](int idx, int16_t v, char* s) { sprintf(s, "%d", clockScaleLut[v]); };
            globalMenu.Formatters[3] = [gateFilters](int idx, int16_t v, char* s) { strcpy(s, gateFilters[v]); };
            for (int i=4; i<=8; i++)
                globalMenu.Formatters[i] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };

            globalMenu.SetLength(9);
            globalMenu.SelectedItem = 0;
            globalMenu.TopItem = 0;
            globalMenu.EnableSelection = true;
            globalMenu.QuadMode = false;

            globalMenu.ValueChangedCallback = [gateFilterValues](int idx, int16_t value)
            {
                if (idx == 3)
                    inProcessor.GateSpeed = gateFilterValues[value];
            };

            globalMenu.HandleEncoderSwitchCallback = [](Menu* menu, bool value)
            {
                if (menu->SelectedItem == 7)
                    ActiveMenu = &calibrateMenu;
                else if (menu->SelectedItem == 8)
                    ActiveMenu = &scopeMenu;
                else
                    HandleEncoderSwitchDefault(menu, value);
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

            calibrateMenu.Min[0] = -256;
            calibrateMenu.Min[1] = -256;
            calibrateMenu.Min[2] = -256;
            calibrateMenu.Min[3] = -256;
            calibrateMenu.Min[4] = -256;
            calibrateMenu.Min[5] = -256;
            calibrateMenu.Min[6] = -256;
            calibrateMenu.Min[7] = -256;

            calibrateMenu.Max[0] = 256;
            calibrateMenu.Max[1] = 256;
            calibrateMenu.Max[2] = 256;
            calibrateMenu.Max[3] = 256;
            calibrateMenu.Max[4] = 256;
            calibrateMenu.Max[5] = 256;
            calibrateMenu.Max[6] = 256;
            calibrateMenu.Max[7] = 256;

            calibrateMenu.Min[8] = 800;
            calibrateMenu.Min[9] = 800;
            calibrateMenu.Min[10] = 800;
            calibrateMenu.Min[11] = 800;
            calibrateMenu.Min[12] = 800;
            calibrateMenu.Min[13] = 800;
            calibrateMenu.Min[14] = 800;
            calibrateMenu.Min[15] = 800;

            calibrateMenu.Max[8] = 1200;
            calibrateMenu.Max[9] = 1200;
            calibrateMenu.Max[10] = 1200;
            calibrateMenu.Max[11] = 1200;
            calibrateMenu.Max[12] = 1200;
            calibrateMenu.Max[13] = 1200;
            calibrateMenu.Max[14] = 1200;
            calibrateMenu.Max[15] = 1200;

            calibrateMenu.Values[8] = 1000;
            calibrateMenu.Values[9] = 1000;
            calibrateMenu.Values[10] = 1000;
            calibrateMenu.Values[11] = 1000;
            calibrateMenu.Values[12] = 1000;
            calibrateMenu.Values[13] = 1000;
            calibrateMenu.Values[14] = 1000;
            calibrateMenu.Values[15] = 1000;

            for (int i = 8; i < 16; i++)
                calibrateMenu.Formatters[i] = [](int idx, int16_t v, char* s) { sprintf(s, "%.3f", v*0.001); };

            calibrateMenu.ValueChangedCallback = [](int idx, float v)
            {
                if (idx < 4) inProcessor.OffsetCv[idx] = v;
                else if (idx < 8) inProcessor.OffsetMod[idx-4] = v;
                else if (idx < 12) inProcessor.ScaleCv[idx-8] = v*0.001;
                else if (idx < 16) inProcessor.ScaleMod[idx-12] = v*0.001;
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
            voiceMenu.Captions[2] = "Pitch Offset";
            //voiceMenu.Captions[x] = "> Load Preset";
            //voiceMenu.Captions[x] = "> Save Preset";
            voiceMenu.Captions[3] = "> Init Voice";
            voiceMenu.Captions[4] = "> Clear All Mods";
            voiceMenu.Captions[5] = "Midi Ch";
            voiceMenu.Captions[6] = "Audio In L";
            voiceMenu.Captions[7] = "Audio In R";
            voiceMenu.Captions[8] = "Audio Out L";
            voiceMenu.Captions[9] = "Audio Out R";
            voiceMenu.Captions[10] = "CV In";
            voiceMenu.Captions[11] = "Gate In";

            voiceMenu.Values[6] = 1;
            voiceMenu.Values[7] = 1;
            voiceMenu.Values[8] = 1;
            voiceMenu.Values[9] = 1;
            voiceMenu.Values[10] = 1;
            voiceMenu.Values[11] = 1;

            voiceMenu.Max[0] = 24;
            voiceMenu.Max[1] = 24;
            voiceMenu.Max[2] = 48;
            voiceMenu.Max[5] = 17;
            voiceMenu.Max[6] = 4;
            voiceMenu.Max[7] = 4;
            voiceMenu.Max[8] = 4;
            voiceMenu.Max[9] = 4;
            voiceMenu.Max[10] = 4;
            voiceMenu.Max[11] = 4;

            voiceMenu.Min[2] = -48;
            voiceMenu.Min[6] = 1;
            voiceMenu.Min[7] = 1;
            voiceMenu.Min[8] = 1;
            voiceMenu.Min[9] = 1;
            voiceMenu.Min[10] = 1;
            voiceMenu.Min[11] = 1;

            voiceMenu.Formatters[0] = [](int idx, int16_t v, char* s) { sprintf(s, "%ddB", -12+v); };
            voiceMenu.Formatters[1] = [](int idx, int16_t v, char* s) { sprintf(s, "%ddB", -12+v); };
            voiceMenu.Formatters[3] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };
            voiceMenu.Formatters[4] = [](int idx, int16_t v, char* s) { strcpy(s, ""); };
            voiceMenu.Formatters[5] = [](int idx, int16_t v, char* s) 
            { 
                if (v == 0) strcpy(s, "Off");
                else if (v == 1) strcpy(s, "Omni");
                else sprintf(s, "%d", v-1); 
            };

            voiceMenu.SetLength(12);
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

        void BuildGeneratorSelectMenu()
        {
            generatorSelectMenu.SetLength(1);
            generatorSelectMenu.CustomOnlyMode = true;
            generatorSelectMenu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
            {
                YieldAudio();
                int selectedGen = generatorSelectMenu.Values[0];
                if (selectedGen < generatorRegistry.Count)
                    generatorRegistry.SplashScreenBuilders[selectedGen](display);
                YieldAudio();
                if (generatorSelectMenu.EditMode)
                {
                    display->fillTriangle(118, 8, 126, 8, 122, 4, SH110X_WHITE);
                    display->fillTriangle(118, 55, 126, 55, 122, 59, SH110X_WHITE);
                }
            };
            generatorSelectMenu.HandleEncoderCallback = [](Menu* menu, int tick)
            {
                LogInfo("Bang!");
                bool fxOnly = menu->Values[1] == 1;
                int currentId = menu->Values[0];

                if (!fxOnly)
                    menu->TickValue(menu->SelectedItem, tick);
                else
                {
                    LogInfof("currentId is %d", currentId);

                    int newId = tick > 0 
                        ? generatorRegistry.GetNextInsertFxIndex(currentId) 
                        : generatorRegistry.GetPrevInsertFxIndex(currentId);
                    if (newId == -1)
                    {
                        LogInfo("NewId is -1!");
                    }
                    else
                    {
                        LogInfof("Setting newId to %d", newId);
                        menu->SetValue(0, newId);
                    }
                }
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
            BuildGeneratorSelectMenu();
        }

        
    }
}
