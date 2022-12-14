// #include "redux.h"
// #include "menu.h"

// namespace Cyber
// {
//     const int RATE = 0;
//     const int MIX = 1;
//     const int GAIN = 2;
//     const int BITCRUSH = 3;
    
//     Redux::Redux()
//     {
//         menu.Captions[RATE] = "Rate Reduce";
//         menu.Captions[MIX] = "Mix";
//         menu.Captions[GAIN] = "Gain";
//         menu.Captions[BITCRUSH] = "Bitcrush";
        
//         menu.Values[RATE] = 0;
//         menu.Values[MIX] = 0.5;
//         menu.Values[BITCRUSH] = 1;
//         menu.Values[GAIN] = 0.5;

//         for (int i = 0; i < 4; i++)
//         {
//             menu.Formatters[i] = [this](int idx, float value, int sv, char* target)
//             {
//                 float val = GetScaledParameter(idx);
//                 sprintf(target, "%.1f", val);
//             };
//         }
        
//         menu.SetLength(4);
//         menu.SelectedItem = 0;
//         menu.TopItem = 0;
//         menu.EnableSelection = false;
//         menu.QuadMode = true;
//     }

//     float Redux::GetScaledParameter(int idx)
//     {
//         if (idx == RATE) return 1.0f + Utils::Resp3dec(menu.Values[RATE]) * 63.0f;
//         if (idx == MIX) return menu.Values[MIX];
//         if (idx == GAIN) return -12.0f + 24.0f * menu.Values[GAIN];
//         if (idx == BITCRUSH) return 1.0f + menu.Values[BITCRUSH] * 11.0f;
//         return 0;
//     }


//     Menu* Redux::GetMenu()
//     {
//         return &menu;
//     }

//     void Redux::Process(GeneratorArgs args)
//     {
//         float rate = GetScaledParameter(RATE);
//         float bits = GetScaledParameter(BITCRUSH);
//         float gain = Utils::DB2Gainf(GetScaledParameter(GAIN));
//         float mix = GetScaledParameter(MIX);

//         float levels = powf(2, bits);
//         float levelsInv = 1.0f/levels;

//         for (int i = 0; i < args.Size; i++)
//         {
//             float s = args.InputLeft[i];

//             // rate reduce
//             if (rate > 1)
//             {
//                 phasor += 1.0f;
//                 if (phasor > rate)
//                 {
//                     phasor -= rate;
//                     sample = args.InputLeft[i];
//                 }
//                 s = sample;
//             }

//             s = s * mix + args.InputLeft[i] * (1-mix);
//             s *= gain;

//             // bitcrush
//             if (rate < 12)
//                 s = ((int)(s * levels))*levelsInv;

//             args.OutputLeft[i] = s;
//         }
//     }
// }
