#include "cyberpsycho.h"

// Definitions for singleton objects.

AudioIo audio;
InputProcessor inProcessor;
MenuManager menuManager;
ControlManager controls;
I2CMaster& master = Master;