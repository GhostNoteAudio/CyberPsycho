#include "menus.h"
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
        Menu pitchTrigMenu;
        Menu generatorSelectMenu;

        void BuildScopeMenu()
        {
            scopeMenu.SetLength(4);
            scopeMenu.CustomOnlyMode = true;
            scopeMenu.RenderCustomDisplayCallback = [](U8G2* display)
            {
                display->clearDisplay();
                float h = display->getHeight();
                display->setDrawColor(1);

                for (int x = 0; x < display->getWidth(); x++)
                {
                    if (x % 16 == 0)
                        YieldAudio();
                    uint8_t y = Scope::data[x] >> 6; // 12 bit to 6 bit (0-63)
                    display->drawPixel(x, (h-1) - y);
                }

                display->setFont(DEFAULT_FONT);
                display->setCursor(65, 63);
                char readout[16];
                auto readoutVal = scopeMenu.Values[3];
                YieldAudio();

                if (readoutVal == 1)
                {
                    display->setDrawColor(1);
                    display->drawBox(64, 55, 60, 9);
                    display->setDrawColor(0);
                    sprintf(readout, "Min:%d", Utils::Min(Scope::data, 128));
                    display->println(readout);
                }
                else if (readoutVal == 2)
                {
                    display->setDrawColor(1);
                    display->drawBox(64, 55, 60, 9);
                    display->setDrawColor(0);
                    sprintf(readout, "Max:%d", Utils::Max(Scope::data, 128));
                    display->println(readout);
                }
                else if (readoutVal == 3)
                {
                    display->setDrawColor(1);
                    display->drawBox(64, 55, 60, 9);
                    display->setDrawColor(0);
                    sprintf(readout, "Mean:%d", (int)Utils::Mean(Scope::data, 128));
                    display->println(readout);
                }
                YieldAudio();

                bool shouldDisplayChange = (millis() - lastChangeMillis) < 2000;
                if (shouldDisplayChange)
                {
                    display->setDrawColor(1);
                    display->drawBox(0, 55, 60, 9);
                    display->setCursor(1, 63);
                    char readout[16];
                    int dsEffective = (1<<Scope::downsampling);
                    display->setDrawColor(0);

                    if (lastChangeIdx == 0) sprintf(readout, "Channel:%d", Scope::channel);
                    if (lastChangeIdx == 1) sprintf(readout, "Divide:%d", dsEffective);
                    if (lastChangeIdx == 2) sprintf(readout, "Freq:%d", Scope::triggerFreq);
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
            initMenu.RenderCustomDisplayCallback = [](U8G2* display)
            {
                display->clearDisplay();
                YieldAudio();
                display->setFont(DEFAULT_FONT);
                display->setDrawColor(1);
                int w = initMenu.GetStringWidth(display, "Cyberpsycho");
                int offset = (display->getWidth() - w) / 2;
                display->setCursor(offset, 22);
                display->print("Cyberpsycho");
                YieldAudio();
                
                w = initMenu.GetStringWidth(display, "Ghost Note Audio");
                offset = (display->getWidth() - w) / 2;
                display->setCursor(offset, 38);
                display->print("Ghost Note Audio");
                YieldAudio();

                w = initMenu.GetStringWidth(display, VERSION);
                offset = (display->getWidth() - w) / 2;
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

        void BuildPitchTrigMenu()
        {
            pitchTrigMenu.SetLength(0);
            pitchTrigMenu.CustomOnlyMode = true;
            pitchTrigMenu.RenderCustomDisplayCallback = [](U8G2* display)
            {
                display->clearDisplay();
                display->setFont(DEFAULT_FONT);
                display->setDrawColor(1);
                display->setCursor(30, 30);
                display->println("Coming Soon");
            };
        }

        void BuildGeneratorSelectMenu()
        {
            generatorSelectMenu.SetLength(1);
            generatorSelectMenu.CustomOnlyMode = true;
            generatorSelectMenu.RenderCustomDisplayCallback = [](U8G2* display)
            {
                YieldAudio();
                int selectedGen = generatorSelectMenu.Values[0];
                if (selectedGen < generatorRegistry.Count)
                    generatorRegistry.SplashScreenBuilders[selectedGen](display);
                YieldAudio();
                if (generatorSelectMenu.EditMode)
                {
                    display->setDrawColor(1);
                    display->drawTriangle(118, 8, 126, 8, 122, 4);
                    display->drawTriangle(118, 55, 126, 55, 122, 59);
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
            BuildPitchTrigMenu();
            BuildGeneratorSelectMenu();
        }

        
    }
}
