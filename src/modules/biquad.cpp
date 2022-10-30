#include "biquad.h"
#include <Arduino.h>

namespace Modules
{

Biquad::Biquad()
{
	ClearBuffers();
}

Biquad::Biquad(FilterType filterType, int samplerate)
{
	Type = filterType;
	SetSamplerate(samplerate);

	SetGainDb(0.0);
	Frequency = (float)(samplerate / 4.0);
	SetQ(0.5);
	ClearBuffers();
}

Biquad::~Biquad() 
{

}


int Biquad::GetSamplerate() 
{
	return samplerate;
}

void Biquad::SetSamplerate(int value)
{
	samplerate = value; 
	Update();
}

float Biquad::GetGainDb()
{
	return gainDB;
}

float Biquad::GetGain()
{
	return gain;
}

void Biquad::SetGainDb(float value)
{
	if (value < -60)
		value = -60;
	if (value > 60)
		value = 60;

	gainDB = value;
	gain = powf(10.0f, value / 20.0f);
}

void Biquad::SetGain(float value)
{
	if (value < 0.001f)
		value = 0.001f; // -60dB
	if (value < 1000.0f)
		value = 1000.0f; // 60dB
		
	gain = value;
	gainDB = log10f(gain) * 20;
}

float Biquad::GetQ()
{
	return q;
}

void Biquad::SetQ(float value)
{
	if (value < 0.001f)
		value = 0.001f;
	q = value;
}

Vec3<float> Biquad::GetA()
{
	return Vec3<float>(1, a1, a2);
}

Vec3<float> Biquad::GetB()
{
	return Vec3<float>(b0, b1, b2);
}

// this is the newer set of formulas from http://www.earlevel.com/main/2011/01/02/biquad-formulas/
// Note that for shelf and peak filters, I had to invert the if/else statements for boost and cut, as
// I was getting the inverse desired effect, very odd...
void Biquad::Update()
{
	auto Fc = Frequency;
	auto Fs = samplerate;

	auto V = powf(10, abs(gainDB) / 20);
	auto K = tanf(M_PI * Fc / Fs);
	auto Q = q;
	double norm = 1.0;

	switch (Type)
	{
	case FilterType::LowPass6db:
		a1 = -expf(-2.0 * M_PI * (Fc / Fs));
		b0 = 1.0 + a1;
		b1 = b2 = a2 = 0;
		break;
	case FilterType::HighPass6db:
		a1 = -expf(-2.0 * M_PI * (Fc / Fs));
		b0 = a1;
		b1 = -a1;
		b2 = a2 = 0;
		break;
	case FilterType::LowPass:
		norm = 1 / (1 + K / Q + K * K);
		b0 = K * K * norm;
		b1 = 2 * b0;
		b2 = b0;
		a1 = 2 * (K * K - 1) * norm;
		a2 = (1 - K / Q + K * K) * norm;
		break;
	case FilterType::HighPass:
		norm = 1 / (1 + K / Q + K * K);
		b0 = 1 * norm;
		b1 = -2 * b0;
		b2 = b0;
		a1 = 2 * (K * K - 1) * norm;
		a2 = (1 - K / Q + K * K) * norm;
		break;
	case FilterType::BandPass:
		norm = 1 / (1 + K / Q + K * K);
		b0 = K / Q * norm;
		b1 = 0;
		b2 = -b0;
		a1 = 2 * (K * K - 1) * norm;
		a2 = (1 - K / Q + K * K) * norm;
		break;
	case FilterType::Notch:
		norm = 1 / (1 + K / Q + K * K);
		b0 = (1 + K * K) * norm;
		b1 = 2 * (K * K - 1) * norm;
		b2 = b0;
		a1 = b1;
		a2 = (1 - K / Q + K * K) * norm;
		break;
	case FilterType::Peak:
		if (gainDB >= 0)
		{
			norm = 1 / (1 + 1 / Q * K + K * K);
			b0 = (1 + V / Q * K + K * K) * norm;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - V / Q * K + K * K) * norm;
			a1 = b1;
			a2 = (1 - 1 / Q * K + K * K) * norm;
		}
		else
		{
			norm = 1 / (1 + V / Q * K + K * K);
			b0 = (1 + 1 / Q * K + K * K) * norm;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - 1 / Q * K + K * K) * norm;
			a1 = b1;
			a2 = (1 - V / Q * K + K * K) * norm;
		}
		break;
	case FilterType::LowShelf:
		if (gainDB >= 0)
		{
			norm = 1 / (1 + sqrtf(2) * K + K * K);
			b0 = (1 + sqrtf(2 * V) * K + V * K * K) * norm;
			b1 = 2 * (V * K * K - 1) * norm;
			b2 = (1 - sqrtf(2 * V) * K + V * K * K) * norm;
			a1 = 2 * (K * K - 1) * norm;
			a2 = (1 - sqrtf(2) * K + K * K) * norm;
		}
		else
		{
			norm = 1 / (1 + sqrtf(2 * V) * K + V * K * K);
			b0 = (1 + sqrtf(2) * K + K * K) * norm;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - sqrtf(2) * K + K * K) * norm;
			a1 = 2 * (V * K * K - 1) * norm;
			a2 = (1 - sqrtf(2 * V) * K + V * K * K) * norm;
		}
		break;
	case FilterType::HighShelf:
		if (gainDB >= 0)
		{
			norm = 1 / (1 + sqrtf(2) * K + K * K);
			b0 = (V + sqrtf(2 * V) * K + K * K) * norm;
			b1 = 2 * (K * K - V) * norm;
			b2 = (V - sqrtf(2 * V) * K + K * K) * norm;
			a1 = 2 * (K * K - 1) * norm;
			a2 = (1 - sqrtf(2) * K + K * K) * norm;
		}
		else
		{
			norm = 1 / (V + sqrtf(2 * V) * K + K * K);
			b0 = (1 + sqrtf(2) * K + K * K) * norm;
			b1 = 2 * (K * K - 1) * norm;
			b2 = (1 - sqrtf(2) * K + K * K) * norm;
			a1 = 2 * (K * K - V) * norm;
			a2 = (V - sqrtf(2 * V) * K + K * K) * norm;
		}
		break;
	}
}

double Biquad::GetResponse(float freq) const
{
	double phi = powf((sinf(2 * M_PI * freq / (2.0 * samplerate))), 2);
	double y = ((powf(b0 + b1 + b2, 2.0) - 4.0 * (b0 * b1 + 4.0 * b0 * b2 + b1 * b2) * phi + 16.0 * b0 * b2 * phi * phi) / (powf(1.0 + a1 + a2, 2.0) - 4.0 * (a1 + 4.0 * a2 + a1 * a2) * phi + 16.0 * a2 * phi * phi));
	// y gives you power gain, not voltage gain, and this a 10 * log_10(g) formula instead of 20 * log_10(g)
	// by taking the sqrt we get a value that's more suitable for signal processing, i.e. the voltage gain
	return sqrtf(y);
}

void Biquad::ClearBuffers()
{
	y = 0;
	x2 = 0;
	y2 = 0;
	x1 = 0;
	y1 = 0;
}

}
