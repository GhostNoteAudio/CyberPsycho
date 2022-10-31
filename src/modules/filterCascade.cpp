#include "filterCascade.h"
#include "utils.h"

using namespace Cyber;

namespace Modules
{
	float FilterCascade::CVtoAlpha[CVtoAlphaSize];

	void FilterCascade::ComputeCVtoAlpha()
	{
		double fsInv = 1.0 / (SAMPLERATE * Oversample);
		for (int i = 0; i < CVtoAlphaSize; i++)
		{
			double freq = 440.0 * powf(2, (i - 69.0) / 12);
			CVtoAlpha[i] = (float)((1.0f - 2.0f * freq * fsInv) * (1.0f - 2.0f * freq * fsInv));
		}
	}

	FilterCascade::FilterCascade()
	{
		Drive = 0.0;
		Cutoff = 1.0;
		Resonance = 0.0;
		ResonanceMod = 0.0;
		CutoffMod = 0.0;
		DriveMod = 0.0;
		SetMode(InternalFilterMode::Lp24);

        ComputeCVtoAlpha();
		fsinv = 1.0f / (Oversample * SAMPLERATE);
		Cutoff = 1;
		oversampledInput = 0;
		Update();
	}

	void FilterCascade::Process(float * input, int len)
	{
		Update();
		
		for (int i = 0; i < len; i++)
		{
			float value = ProcessSample(input[i]) * gInv;
			buffer[i] = value;
		}
	}

	float FilterCascade::ProcessSample(float input)
	{
		input *= gain;

		for (int i = 0; i < Oversample; i++)
		{
			float in = mx * (i * input + (Oversample - i) * oversampledInput); // linear interpolation
			in = tanhf(in);

			float fb = totalResonance * 4.2f * (feedback - 0.5f * in);
			float val = in - fb;
			x = val;

			// 4 cascaded low pass stages
			a = (1 - p) * val + p * a;
			val = a;
			b = (1 - p) * val + p * b;
			val = b;
			c = (1 - p) * val + p * c;
			val = c;
			d = (1 - p) * val + p * d;
			val = d;

			feedback = tanhf(val);
		}

		oversampledInput = input;
		float sample = (c0 * x + c1 * a + c2 * b + c3 * c + c4 * d) * (1 - totalResonance * 0.5f);
		return sample * 4; // gain fudge
	}

	void FilterCascade::SetMode(InternalFilterMode mode)
	{
		if (mode < InternalFilterMode::Lp24 || mode >= InternalFilterMode::Count)
			return;

		switch (mode)
		{
		case InternalFilterMode::Lp6:
			c0 = 0.0; c1 = 1.0; c2 = 0.0; c3 = 0.0; c4 = 0.0;
			break;
		case InternalFilterMode::Lp12:
			c0 = 0.0; c1 = 0.0; c2 = 1.0; c3 = 0.0; c4 = 0.0;
			break;
		case InternalFilterMode::Lp18:
			c0 = 0.0; c1 = 0.0; c2 = 0.0; c3 = 1.0; c4 = 0.0;
			break;
		case InternalFilterMode::Lp24:
			c0 = 0.0; c1 = 0.0; c2 = 0.0; c3 = 0.0; c4 = 1.0;
			break;
		case InternalFilterMode::Hp6:
			c0 = 1.0; c1 = -1.0; c2 = 0.0; c3 = 0.0; c4 = 0.0;
			break;
		case InternalFilterMode::Hp12:
			c0 = 1.0; c1 = -2.0; c2 = 1.0; c3 = 0.0; c4 = 0.0;
			break;
		case InternalFilterMode::Hp18:
			c0 = 1.0; c1 = -3.0; c2 = 3.0; c3 = -1.0; c4 = 0.0;
			break;
		case InternalFilterMode::Hp24:
			c0 = 1.0; c1 = -4.0; c2 = 6.0; c3 = -4.0; c4 = 1.0;
			break;
		case InternalFilterMode::Bp12_12:
			c0 = 0.0; c1 = 0.0; c2 = 1.0; c3 = -2.0; c4 = 1.0;
			break;
		case InternalFilterMode::Bp6_18:
			c0 = 0.0; c1 = 0.0; c2 = 0.0; c3 = 1.0; c4 = -1.0;
			break;
		case InternalFilterMode::Bp18_6:
			c0 = 0.0; c1 = 1.0; c2 = -3.0; c3 = 3.0; c4 = -1.0;
			break;
		case InternalFilterMode::Bp6_12:
			c0 = 0.0; c1 = 0.0; c2 = 1.0; c3 = -1.0; c4 = 0.0;
			break;
		case InternalFilterMode::Bp12_6:
			c0 = 0.0; c1 = 1.0; c2 = -2.0; c3 = 1.0; c4 = 0.0;
			break;
		case InternalFilterMode::Bp6_6:
			c0 = 0.0; c1 = 1.0; c2 = -1.0; c3 = 0.0; c4 = 0.0;
			break;
        default:
            break;
		}
	}

	void FilterCascade::Update()
	{
		float driveTotal = Drive + DriveMod;
		driveTotal = Utils::Limit(driveTotal, 0.0f, 1.0f);

		gain = (0.1f + 2.0f * driveTotal * driveTotal);

		totalResonance = Resonance + ResonanceMod;
		totalResonance = Utils::Limit(totalResonance, 0.0f, 1.0f);
		totalResonance = Utils::Resp2oct(totalResonance) * 0.999f;

		// Voltage is 1V/OCt, C0 = 16.3516Hz
		float noteNum = Cutoff + CutoffMod;
		noteNum = Utils::Limit(noteNum, 0.0f, 136.0f);

		p = CVtoAlpha[(int)noteNum];

		gInv = sqrt(1.0f / gain);
		mx = 1.0f / Oversample;
	}

}
