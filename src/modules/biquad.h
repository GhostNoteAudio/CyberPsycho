#pragma once

namespace Modules
{
	template<typename T>
	struct Vec3
	{
		T Data[3];

		Vec3(T a, T b, T c)
		{
			Data[0] = a;
			Data[1] = b;
			Data[2] = c;
		}
	};

	class Biquad
	{
	public:
		enum class FilterType
		{
			LowPass6db = 0,
			HighPass6db,
			LowPass,
			HighPass,
			BandPass,
			Notch,
			Peak,
			LowShelf,
			HighShelf
		};

	private:
		float fs;
		float fsInv;
		float gainDB;
		float q;
		float a0, a1, a2, b0, b1, b2;
		float x1, x2, y, y1, y2;
		float gain;

	public:
		FilterType Type;
		float Output;
		float Frequency;

		Biquad();
		Biquad(FilterType filterType, float fs);
		~Biquad();

		float GetSamplerate();
		void SetSamplerate(float fs);
		float GetGainDb();
		void SetGainDb(float value);
		float GetGain();
		void SetGain(float value);
		float GetQ();
		void SetQ(float value);
		Vec3<float> GetA();
		Vec3<float> GetB();

		void Update();

		double GetResponse(float freq) const;
			
		float inline Process(float x)
		{
			y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
			x2 = x1;
			y2 = y1;
			x1 = x;
			y1 = y;

			Output = y;
			return Output;
		}

		void inline Process(float* input, float* output, int len)
		{
			for (int i = 0; i < len; i++)
			{
				float x = input[i];
				y = ((b0 * x) + (b1 * x1) + (b2 * x2)) - (a1 * y1) - (a2 * y2);
				x2 = x1;
				y2 = y1;
				x1 = x;
				y1 = y;

				output[i] = y;
			}

			Output = y;
		}

		void ClearBuffers();
	};
}
