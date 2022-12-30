#include "filterCascade.h"
#include "utils.h"
#include "logging.h"

using namespace Cyber;

namespace Modules
{
	float FilterCascade::HztoAlpha(float freq)
	{
		double fsInv = 1.0 / (SAMPLERATE * Oversample);
		float k = 1.0f - 2.0f * freq * fsInv;
		return k * k;
	}

	FilterCascade::FilterCascade()
	{
		Drive = 0.0;
		CutoffHz = 20000.0;
		Resonance = 0.0;
		SetMode(InternalFilterMode::Lp24);

		fsinv = 1.0f / (Oversample * SAMPLERATE);
		oversampledInput = 0;
		Update();
	}

	void FilterCascade::Process(float * input, float* output, int len)
	{
		Update();
		
		for (int i = 0; i < len; i++)
			output[i] = ProcessSample(input[i]);
	}

	float FilterCascade::ProcessSample(float input)
	{
		input *= gain;

		for (int i = 0; i < Oversample; i++)
		{
			float in = mx * (i * input + (Oversample - i) * oversampledInput); // linear interpolation
			in = Utils::tanhm(in, 0.1f);

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

			feedback = Utils::tanhm(val, 0.1f);
		}

		oversampledInput = input;
		float sample = (c0 * x + c1 * a + c2 * b + c3 * c + c4 * d) * (1 - totalResonance * 0.5f);
		return sample * gInv;
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
		float driveTotal = Utils::Clamp(Drive);
		gain = (0.1f + 2.0f * driveTotal * driveTotal);

		totalResonance = Resonance;
		totalResonance = Utils::Clamp(totalResonance);
		totalResonance = Utils::Resp2oct(totalResonance) * 1.02f;

		p = HztoAlpha(CutoffHz);
		LogInfof("P: %.3f", p);
		gInv = sqrt(1.0f / gain);
		mx = 1.0f / Oversample;
	}

}
