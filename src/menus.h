#pragma once
#include "menu.h"
#include "mod_source.h"

namespace Cyber
{
    namespace Menus
    {
        extern Menu initMenu;
        extern Menu globalMenu;
        extern Menu scopeMenu;
        extern Menu calibrateMenu;
        extern Menu pitchTrigMenu;
        extern Menu generatorSelectMenu;
        extern Menu presetMenu;

        void Init();
    }

    auto saveOverlay = [](U8G2* display)
    {
        display->setFont(DEFAULT_FONT);
        display->setDrawColor(0);
        display->drawBox(20, 20, 128-40, 24);
        display->setDrawColor(1);
        display->drawFrame(20, 20, 128-40, 24);
        int w = display->getStrWidth("Saved");
        display->setCursor(64 - w/2, 35);
        display->print("Saved");
    };

    auto loadOverlay = [](U8G2* display)
    {
        display->setFont(DEFAULT_FONT);
        display->setDrawColor(0);
        display->drawBox(20, 20, 128-40, 24);
        display->setDrawColor(1);
        display->drawFrame(20, 20, 128-40, 24);
        int w = display->getStrWidth("Loaded");
        display->setCursor(64 - w/2, 35);
        display->print("Loaded");
    };

    auto initialisedOverlay = [](U8G2* display)
    {
        display->setFont(DEFAULT_FONT);
        display->setDrawColor(0);
        display->drawBox(20, 20, 128-40, 24);
        display->setDrawColor(1);
        display->drawFrame(20, 20, 128-40, 24);
        int w = display->getStrWidth("Initialised");
        display->setCursor(64 - w/2, 35);
        display->print("Initialised");
    };
}
