#pragma once

#include <math.h>
#include "constants.h"

namespace Modules
{
    enum class InternalFilterMode
	{
		Lp24 = 0, // default
		Lp18,
		Lp12,
		Lp6,

		Hp24,
		Hp18,
		Hp12,
		Hp6,

		Bp6_6,
		Bp6_12,
		Bp12_6,

		Bp12_12,
		Bp6_18,
		Bp18_6,

		Count
	};

	class FilterCascade
	{
	private:
		static const int CVtoAlphaSize = 137;
		static float CVtoAlpha[CVtoAlphaSize];
		static void ComputeCVtoAlpha();
	public:
		static inline float GetCvFreq(float cv)
		{
			// Voltage is 1V/OCt, C0 = 16.3516Hz
			// 10.3V = Max = 20614.33hz
			float freq = (float)(440.0f * powf(2, (cv * 12 - 69.0 + 12) / 12));
			return freq;
		}

	public:
		const static int Oversample = 2;

		float Drive = 0;
		float Cutoff = 0;
		float Resonance = 0;
		float c0, c1, c2, c3, c4 = 0;

	private:
		float gain = 1;
		float totalResonance = 0;
		float oversampledInput = 0;

		float p = 0;
		float x = 0;
		float a = 0;
		float b = 0;
		float c = 0;
		float d = 0;
		float feedback = 0;

		float fsinv = 1;
		float gInv = 1;
		float mx = 0;

	public:
		FilterCascade();
		void Process(float* input, float* output, int len);
		void SetMode(InternalFilterMode mode);
		float ProcessSample(float input);
		void Update();
	};
}

