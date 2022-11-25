#include "bypass.h"
#include "voices.h"

namespace Cyber
{
    Bypass::Bypass()
    {
        menu.CustomOnlyMode = true;
        menu.RenderCustomDisplayCallback = [](Adafruit_SH1106G* display)
        {
            display->clearDisplay();
            display->setCursor(16, 28);
            display->setTextSize(2);
            display->print("Bypassed");

            display->setCursor(58, 52);
            display->print(Voices::GetActiveVoice()->ActiveInsert+1);
        };

        menu.SetLength(0);
        menu.SelectedItem = 0;
        menu.TopItem = 0;
        menu.EnableSelection = false;
        menu.QuadMode = true;
    }
}