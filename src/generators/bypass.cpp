// #include "bypass.h"
// #include "voice.h"

// namespace Cyber
// {
//     Bypass::Bypass()
//     {
//         menu.CustomOnlyMode = true;
//         menu.RenderCustomDisplayCallback = [](U8G2* display)
//         {
//             display->clearDisplay();
//             display->setDrawColor(1);
//             display->setFont(BIG_FONT);
//             int w = display->getStrWidth("Bypassed");
//             display->setCursor(display->getWidth()/2 - w/2, 26);
//             display->print("Bypassed");
//         };

//         menu.SetLength(0);
//         menu.SelectedItem = 0;
//         menu.TopItem = 0;
//         menu.EnableSelection = false;
//         menu.QuadMode = true;
//     }
// }