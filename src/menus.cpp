#include "menus.h"
#include "utils.h"
#include "logging.h"
#include "input_processor.h"
#include "scope.h"
#include "voice.h"
#include "generatorRegistry.h"
#include "display_manager.h"
#include "tempo.h"
#include "storage.h"
#include "audio_io.h"
#include "midi.h"

namespace Cyber
{
    namespace Menus
    {
        int lastChangeMillis = 0;
        int lastChangeIdx = 0;
        float scopeAvg = 0;

        Menu initMenu;
        Menu globalMenu;
        Menu scopeMenu;
        Menu calibrateMenu;
        Menu pitchTrigMenu;
        Menu generatorSelectMenu;
        Menu presetMenu;

        enum GlobalMenuItems
        {
            _GainOut = 0,
            _PitchOffset,
            _ClockSource,
            _ClockScale,
            _BPM,
            _MidiEnabled,
            _PitchbendRange,
            _GateFilter,
            _LoadPreset,
            _SavePreset,
            _InitProgram,
            _Modulations,
            _Calibrate,
            _Scope,
        };

        void InitProgram()
        {
            globalMenu.SetValue(_GainOut, 0.5);
            globalMenu.SetValue(_PitchOffset, 0.5);
            globalMenu.SetValue(_ClockSource, 0);
            globalMenu.SetValue(_ClockScale, 0);
            globalMenu.SetScaledValue(_BPM, 120);
            globalMenu.SetValue(_MidiEnabled, 0);
            globalMenu.SetScaledValue(_PitchbendRange, 2);
            globalMenu.SetValue(_GateFilter, 0);
            voice.matrix.Reset();
            voice.SetGenerator(generatorRegistry.GetGenIndexById("GNA-Quad"), true);

            displayManager.SetOverlay(initialisedOverlay, 1000);
        }

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
                    scopeAvg = scopeAvg * 0.9 + Utils::Mean(Scope::data, 128) * 0.1;
                    display->setDrawColor(1);
                    display->drawBox(64, 55, 60, 9);
                    display->setDrawColor(0);
                    sprintf(readout, "Mean:%d", (int)scopeAvg);
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
                    display->println(readout);
                }
            };
            scopeMenu.HandlePotCallback = [](Menu* menu, int idx, float value)
            {
                if (idx == 0)
                    Scope::channel = (int)(value * 7.99);
                if (idx == 1)
                    Scope::downsampling = (int)(value * 9.99);
                if (idx == 3)
                    menu->SetValue(idx, value);

                if (idx <= 1)
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
            globalMenu.Captions[_GainOut] = "Gain Out";
            globalMenu.Captions[_PitchOffset] = "Pitch Offset";
            globalMenu.Captions[_ClockSource] = "Clock Source";
            globalMenu.Captions[_ClockScale] = "Clock Scale";
            globalMenu.Captions[_BPM] = "BPM";
            globalMenu.Captions[_MidiEnabled] = "Midi";
            globalMenu.Captions[_PitchbendRange] = "Pitchb. Range";
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
            globalMenu.Values[_BPM] = 0.42;
            globalMenu.Values[_MidiEnabled] = 0;
            globalMenu.Values[_PitchbendRange] = 0;
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
            globalMenu.Steps[_MidiEnabled] = 2;
            globalMenu.Steps[_PitchbendRange] = 13;
            globalMenu.Steps[_GateFilter] = 4;

            globalMenu.Min[_BPM] = 20;

            globalMenu.Formatters[_BPM] = [](int idx, float v, int sv, char* s)
            {
                if(globalMenu.GetScaledValue(_ClockSource) == 0)
                    sprintf(s, "%d", globalMenu.GetScaledValue(_BPM));
                else
                    sprintf(s, "[%.1f]", tempoState.GetBpm());
            };
            globalMenu.Formatters[_MidiEnabled] = [](int idx, float v, int sv, char* s) { strcpy(s, v < 0.5 ? "Disabled" : "Enabled"); };
            globalMenu.Formatters[_GainOut] = [](int idx, float v, int sv, char* s) { sprintf(s, "%.1fdB", -12.f + sv); };
            globalMenu.Formatters[_ClockSource] = [clockSources](int idx, float v, int sv, char* s) { strcpy(s, clockSources[sv]); };
            globalMenu.Formatters[_ClockScale] = [clockScaleLut](int idx, float v, int sv, char* s) { sprintf(s, "%d", clockScaleLut[sv]); };
            globalMenu.Formatters[_GateFilter] = [gateFilters](int idx, float v, int sv, char* s) { strcpy(s, gateFilters[sv]); };
            for (int i=_LoadPreset; i<=_Scope; i++)
                globalMenu.Formatters[i] = [](int idx, float v, int sv, char* s) { strcpy(s, ""); };

            globalMenu.SetLength(14);
            globalMenu.SelectedItem = 0;
            globalMenu.TopItem = 0;
            globalMenu.QuadMode = false;

            globalMenu.ValueChangedCallback = [gateFilterValues, clockScaleLut](int idx, float value, int sv)
            {
                if (idx == _GainOut)
                {
                    voice.GainOut = Utils::DB2Gainf(-12.0f + sv);
                }
                else if (idx == _PitchOffset)
                {
                    voice.PitchOffset = sv;
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
                else if (idx == _MidiEnabled)
                {
                    midi.MidiEnabled = value >= 0.5;
                }
                else if (idx == _PitchbendRange)
                {
                    midi.PitchbendRange = sv;
                }
            };

            globalMenu.HandleEncoderSwitchCallback = [](Menu* menu, bool value)
            {
                if (!value) return;

                if (menu->SelectedItem == _BPM)
                {
                    if(globalMenu.GetScaledValue(_ClockSource) == 0)
                        HandleEncoderSwitchDefault(menu, value);
                    else
                        return;
                }
                else if (menu->SelectedItem == _InitProgram)
                    InitProgram();
                else if (menu->SelectedItem == _Calibrate)
                    displayManager.ActiveMenu = &calibrateMenu;
                else if (menu->SelectedItem == _Modulations)
                    displayManager.ActiveMenu = voice.matrix.GetMenu();
                else if (menu->SelectedItem == _Scope)
                    displayManager.ActiveMenu = &scopeMenu;
                else if (menu->SelectedItem == _LoadPreset)
                {
                    displayManager.ActiveMenu = &presetMenu;
                    presetMenu.Values[16] = 0; // Load Preset
                }
                else if (menu->SelectedItem == _SavePreset)
                {
                    displayManager.ActiveMenu = &presetMenu;
                    presetMenu.Values[16] = 1; // Save Preset
                }
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

        void BuildPresetMenu()
        {
            presetMenu.SetLength(16);
            presetMenu.Captions[0] = "Preset 1";
            presetMenu.Captions[1] = "Preset 2";
            presetMenu.Captions[2] = "Preset 3";
            presetMenu.Captions[3] = "Preset 4";
            presetMenu.Captions[4] = "Preset 5";
            presetMenu.Captions[5] = "Preset 6";
            presetMenu.Captions[6] = "Preset 7";
            presetMenu.Captions[7] = "Preset 8";
            presetMenu.Captions[8] = "Preset 9";
            presetMenu.Captions[9] = "Preset 10";
            presetMenu.Captions[10] = "Preset 11";
            presetMenu.Captions[11] = "Preset 12";
            presetMenu.Captions[12] = "Preset 13";
            presetMenu.Captions[13] = "Preset 14";
            presetMenu.Captions[14] = "Preset 15";
            presetMenu.Captions[15] = "Preset 16";
            
            auto formatter = [](int idx, float val, int sv, char* dest)
            {
                if (val < 0.5)
                {
                    if (presetMenu.Values[16] == 0)
                        strcpy(dest, "---");
                    else
                        strcpy(dest, "> Save");
                }
                else
                {
                    if (presetMenu.Values[16] == 0)
                        strcpy(dest, "> Load");
                    else
                        strcpy(dest, "> Overwrite");
                }
            };

            for (int i = 0; i < 16; i++)
                presetMenu.Formatters[i] = formatter;

            auto setValues = []()
            {
                DisableAudio disable;
                char presetFile[32];
                char temp[8];

                for (int i = 0; i < 16; i++)
                {
                    strcpy(presetFile, "cyber/presets/preset-");
                    sprintf(temp, "%d", i+1);
                    strcat(presetFile, temp);
                    strcat(presetFile, ".bin");
                    bool exists = Storage::FileExists(presetFile);
                    presetMenu.Values[i] = exists ? 1 : 0;
                }
            };

            setValues();

            presetMenu.HandleEncoderSwitchCallback = [setValues](Menu* menu, bool value)
            {
                if (!value) return;

                bool save = menu->Values[16] == 1;
                bool load = !save;
                int slot = menu->SelectedItem;

                char presetFile[32];
                char temp[8];
                
                if (load)
                {
                    strcpy(presetFile, "cyber/presets/preset-");
                    sprintf(temp, "%d", slot+1);
                    strcat(presetFile, temp);
                    strcat(presetFile, ".bin");
                    LogInfof("Loading preset file: %s", presetFile);
                    Storage::LoadPreset(presetFile, true);
                }
                else
                {
                    strcpy(presetFile, "preset-");
                    sprintf(temp, "%d", slot+1);
                    strcat(presetFile, temp);
                    strcat(presetFile, ".bin");
                    LogInfof("Saving Preset file: %s", presetFile);
                    Storage::SavePreset("cyber/presets", presetFile, true);
                    setValues();
                }

                displayManager.ActiveMenu = &globalMenu;
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
            BuildPresetMenu();
        }

        
    }
}
