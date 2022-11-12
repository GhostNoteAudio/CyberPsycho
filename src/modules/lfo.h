#pragma once
#include "logging.h"
#include "wavetable.h"
#include "lcgRand.h"

namespace Modules
{
    class Lfo
    {
    public:
        enum class Shape
        {
            Sin = 0,
            Pulse,
            Triangle,
            Saw,
            Ramp,
            //RandomStep,
            //RandomSlew
        };

    private:
        bool currentGate;
        uint32_t phasor;
        uint32_t phaseIncrement;

    public:
        float Frequency = 1;
        bool Retrigger = false;
        float StartPhase = 0;
        Shape Waveshape = Shape::Sin;
        bool Unipolar = false;

        inline float Process(bool gate)
        {
            phaseIncrement = Wavetable::GetPhaseIncrement(Frequency);
            bool trig = !this->currentGate && gate;

            if (trig && Retrigger)
            {
                phasor = Wavetable::GetPhasor(StartPhase);
            }

            phasor += phaseIncrement;
            float output = 0.0f;
            
            if (Waveshape == Shape::Sin)
                output = Wavetable::Sin(phasor);
            if (Waveshape == Shape::Pulse)
                output = Wavetable::Pulse(phasor, 0x7FFFFFFF);
            if (Waveshape == Shape::Triangle)
                output = Wavetable::Triangle(phasor);
            if (Waveshape == Shape::Saw)
                output = Wavetable::Saw(phasor);
            if (Waveshape == Shape::Ramp)
                output = -Wavetable::Saw(phasor);

            this->currentGate = gate;
            return output + (int)Unipolar;
        }

        inline static const char* GetShapeName(int value)
        {
            if (value == (int)Shape::Sin)
                return "Sin";
            if (value == (int)Shape::Pulse)
                return "Pulse";
            if (value == (int)Shape::Triangle)
                return "Triangle";
            if (value == (int)Shape::Saw)
                return "Saw";
            if (value == (int)Shape::Ramp)
                return "Ramp";
            return "";
        }
    };
}
