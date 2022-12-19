#include "menus.h"
#include "utils.h"
#include "logging.h"
#include "input_processor.h"
#include "scope.h"
#include "voice.h"
#include "generatorRegistry.h"
#include "display_manager.h"
#include "tempo.h"

namespace Cyber
{
    namespace Menus
    {
        int lastChangeMillis = 0;
        int lastChangeIdx = 0;

        Menu initMenu;
        Menu globalMenu;
        Menu scopeMenu;
        Menu calibrateMenu;
        Menu pitchTrigMenu;
        Menu generatorSelectMenu;

        void BuildScopeMenu()
        {
            scopeMenu.SetLength(4);
            scopeMenu.Steps[3] = 4;
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
                auto readoutVal = scopeMenu.GetScaledValue(3);
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
                    menu->SetValue(idx, value);

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

        enum GlobalMenuItems
        {
            _GainOut = 0,
            _PitchOffset,
            _ClockSource,
            _ClockScale,
            _BPM,
            _GateFilter,
            _LoadPreset,
            _SavePreset,
            _InitProgram,
            _Modulations,
            _Calibrate,
            _Scope,
        };

        void BuildGlobalMenu()
        {
            globalMenu.Captions[_GainOut] = "Gain Out";
            globalMenu.Captions[_PitchOffset] = "Pitch Offset";
            globalMenu.Captions[_ClockSource] = "Clock Source";
            globalMenu.Captions[_ClockScale] = "Clock Scale";
            globalMenu.Captions[_BPM] = "BPM";
            globalMenu.Captions[_GateFilter] = "Gate Filter";
            globalMenu.Captions[_LoadPreset] = "> Load Preset";
            globalMenu.Captions[_SavePreset] = "> Save Preset";
            globalMenu.Captions[_InitProgram] = "> Init Program";
            globalMenu.Captions[_Modulations] = "> Modulations";
            globalMenu.Captions[_Calibrate] = "> Calibrate";
            globalMenu.Captions[_Scope] = "> Scope";

            globalMenu.Values[_GainOut] = 0.5;
            globalMenu.Values[_PitchOffset] = 0.5;
            globalMenu.Values[_ClockSource] = 0;
            globalMenu.Values[_ClockScale] = 0;
            globalMenu.Values[_BPM] = 0.45;
            globalMenu.Values[_GateFilter] = 0;
            globalMenu.Values[_LoadPreset] = 0;
            globalMenu.Values[_SavePreset] = 0;
            globalMenu.Values[_InitProgram] = 0;
            globalMenu.Values[_Modulations] = 0;
            globalMenu.Values[_Calibrate] = 0;
            globalMenu.Values[_Scope] = 0;

            int clockScaleLut[12] = {1,2,4,8,12,16,24,32,48,64,96,128};
            const char* clockSources[_GateFilter] = {"Int", "Ext", "Midi"};
            const char* gateFilters[4] = {"Off", "Mild", "Normal", "High"};
            int gateFilterValues[4] = {255, 128, 64, 16};

            globalMenu.Min[_PitchOffset] = -36;

            globalMenu.Steps[_GainOut] = 25;
            globalMenu.Steps[_PitchOffset] = 36*2+1;
            globalMenu.Steps[_ClockSource] = 3;
            globalMenu.Steps[_ClockScale] = 12;
            globalMenu.Steps[_BPM] = 281;
            globalMenu.Steps[_GateFilter] = 4;

            globalMenu.Min[_BPM] = 20;

            globalMenu.Formatters[_GainOut] = [](int idx, float v, int sv, char* s) { sprintf(s, "%.1fdB", -12.f + sv); };
            globalMenu.Formatters[_ClockSource] = [clockSources](int idx, float v, int sv, char* s) { strcpy(s, clockSources[sv]); };
            globalMenu.Formatters[_ClockScale] = [clockScaleLut](int idx, float v, int sv, char* s) { sprintf(s, "%d", clockScaleLut[sv]); };
            globalMenu.Formatters[_GateFilter] = [gateFilters](int idx, float v, int sv, char* s) { strcpy(s, gateFilters[sv]); };
            for (int i=_LoadPreset; i<=_Scope; i++)
                globalMenu.Formatters[i] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };

            globalMenu.SetLength(12);
            globalMenu.SelectedItem = 0;
            globalMenu.TopItem = 0;
            globalMenu.QuadMode = false;

            globalMenu.ValueChangedCallback = [gateFilterValues, clockScaleLut](int idx, float value, int sv)
            {
                if (idx == _GainOut)
                {
                    voice.GainOut[0] = Utils::DB2Gainf(-12.0f + sv);
                    voice.GainOut[1] = Utils::DB2Gainf(-12.0f + sv);
                    voice.GainOut[2] = Utils::DB2Gainf(-12.0f + sv);
                    voice.GainOut[3] = Utils::DB2Gainf(-12.0f + sv);
                }
                else if (idx == _PitchOffset)
                {
                    voice.PitchOffset[0] = sv;
                    voice.PitchOffset[1] = sv;
                    voice.PitchOffset[2] = sv;
                    voice.PitchOffset[3] = sv;
                }
                else if (idx == _GateFilter)
                {
                    inProcessor.GateSpeed = gateFilterValues[sv];
                }
                else if (idx == _BPM)
                {
                    tempoState.SetInternalBpm(sv);
                }
                else if (idx == _ClockScale)
                {
                    tempoState.SetTrigDivision(clockScaleLut[sv]);
                }
                else if (idx == _ClockSource)
                {
                    tempoState.SetTempoMode((TempoMode)sv);
                }
            };

            globalMenu.HandleEncoderSwitchCallback = [](Menu* menu, bool value)
            {
                if (menu->SelectedItem == _Calibrate)
                    displayManager.ActiveMenu = &calibrateMenu;
                else if (menu->SelectedItem == _Modulations)
                    displayManager.ActiveMenu = voice.matrix.GetMenu();
                else if (menu->SelectedItem == _Scope)
                    displayManager.ActiveMenu = &scopeMenu;
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
                calibrateMenu.Formatters[i] = [](int idx, float v, int sv, char* s) { sprintf(s, "%.3f", sv*0.001); };

            calibrateMenu.ValueChangedCallback = [](int idx, float v, int sv)
            {
                if (idx < 4) inProcessor.OffsetCv[idx] = sv;
                else if (idx < 8) inProcessor.OffsetMod[idx-4] = sv;
                else if (idx < 12) inProcessor.ScaleCv[idx-8] = sv*0.001;
                else if (idx < 16) inProcessor.ScaleMod[idx-12] = sv*0.001;
            };

            calibrateMenu.SetLength(16);
            calibrateMenu.SelectedItem = 0;
            calibrateMenu.TopItem = 0;
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
                if (selectedGen < generatorRegistry.GetGenCount())
                    generatorRegistry.GetSplashScreenBuilder(selectedGen)(display);
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
                // abusing the menu system to pass extra parameters
                int currentId = menu->Values[0];
                int maxCount = menu->Values[1];
                
                int newId = currentId + tick;
                if (newId < 0) newId = 0;
                if (newId >= maxCount) newId = maxCount - 1;
                menu->Values[0] = newId;
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
