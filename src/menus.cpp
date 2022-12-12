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

            globalMenu.Steps[0] = 3;
            globalMenu.Steps[1] = 12;
            globalMenu.Steps[2] = 280;
            globalMenu.Steps[3] = 4;

            globalMenu.Min[2] = 20;

            globalMenu.Formatters[0] = [clockSources](int idx, float v, char* s) { strcpy(s, clockSources[globalMenu.GetScaledValue(idx)]); };
            globalMenu.Formatters[1] = [clockScaleLut](int idx, float v, char* s) { sprintf(s, "%d", clockScaleLut[globalMenu.GetScaledValue(idx)]); };
            globalMenu.Formatters[3] = [gateFilters](int idx, float v, char* s) { strcpy(s, gateFilters[globalMenu.GetScaledValue(idx)]); };
            for (int i=4; i<=8; i++)
                globalMenu.Formatters[i] = [](int idx, float v, char* s) { strcpy(s, ""); };

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

            calibrateMenu.Steps[0] = 513;
            calibrateMenu.Steps[1] = 513;
            calibrateMenu.Steps[2] = 513;
            calibrateMenu.Steps[3] = 513;
            calibrateMenu.Steps[4] = 513;
            calibrateMenu.Steps[5] = 513;
            calibrateMenu.Steps[6] = 513;
            calibrateMenu.Steps[7] = 513;

            calibrateMenu.Min[8] = 800;
            calibrateMenu.Min[9] = 800;
            calibrateMenu.Min[10] = 800;
            calibrateMenu.Min[11] = 800;
            calibrateMenu.Min[12] = 800;
            calibrateMenu.Min[13] = 800;
            calibrateMenu.Min[14] = 800;
            calibrateMenu.Min[15] = 800;

            calibrateMenu.Steps[8] = 401;
            calibrateMenu.Steps[9] = 401;
            calibrateMenu.Steps[10] = 401;
            calibrateMenu.Steps[11] = 401;
            calibrateMenu.Steps[12] = 401;
            calibrateMenu.Steps[13] = 401;
            calibrateMenu.Steps[14] = 401;
            calibrateMenu.Steps[15] = 401;

            calibrateMenu.Values[0] = 0.5f;
            calibrateMenu.Values[1] = 0.5f;
            calibrateMenu.Values[2] = 0.5f;
            calibrateMenu.Values[3] = 0.5f;
            calibrateMenu.Values[4] = 0.5f;
            calibrateMenu.Values[5] = 0.5f;
            calibrateMenu.Values[6] = 0.5f;
            calibrateMenu.Values[7] = 0.5f;
            calibrateMenu.Values[8] = 0.5f;
            calibrateMenu.Values[9] = 0.5f;
            calibrateMenu.Values[10] = 0.5f;
            calibrateMenu.Values[11] = 0.5f;
            calibrateMenu.Values[12] = 0.5f;
            calibrateMenu.Values[13] = 0.5f;
            calibrateMenu.Values[14] = 0.5f;
            calibrateMenu.Values[15] = 0.5f;

            for (int i = 8; i < 16; i++)
                calibrateMenu.Formatters[i] = [](int idx, float v, char* s) { sprintf(s, "%.3f", calibrateMenu.GetScaledValue(idx)*0.001); };

            calibrateMenu.ValueChangedCallback = [](int idx, float v)
            {
                auto sv = calibrateMenu.GetScaledValue(idx);
                if (idx < 4) inProcessor.OffsetCv[idx] = sv;
                else if (idx < 8) inProcessor.OffsetMod[idx-4] = sv;
                else if (idx < 12) inProcessor.ScaleCv[idx-8] = sv*0.001;
                else if (idx < 16) inProcessor.ScaleMod[idx-12] = sv*0.001;
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
                // abusing the menu system to pass extra parameters
                int currentId = menu->Values[0];
                bool fxOnly = menu->Values[1] == 1;
                int maxCount = menu->Values[3];
                
                if (!fxOnly)
                {
                    int newId = currentId + tick;
                    
                    if (newId < 0) newId = 0;
                    if (newId >= maxCount) newId = maxCount - 1;
                    menu->SetValue(0, newId);
                }
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
