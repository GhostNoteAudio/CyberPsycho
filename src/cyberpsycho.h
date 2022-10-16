#include "Arduino.h"
#include "display_manager.h"
#include "controls.h"
#include "audio_io.h"
#include "input_processor.h"
#include <i2c_driver_wire.h>
#include "audio_yield.h"

#include "periodic_execution.h"
#include "menu.h"
#include "menus.h"
#include "counter.h"
#include "logging.h"
#include "utils.h"
#include "timers.h"

namespace Cyber
{
    extern AudioIo audio;
    extern InputProcessor inProcessor;
    extern DisplayManager displayManager;
    extern ControlManager controls;
    extern I2CMaster& i2cMaster;
    extern ModalState modalState;

    namespace Scope
    {
        extern uint16_t data[128];
        extern uint8_t channel;
        extern uint8_t downsampling;
        extern uint16_t triggerFreq;
        extern void ProcessScope(DataBuffer* data);
    }
}