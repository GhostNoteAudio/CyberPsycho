#pragma once

#include <stdint.h>

namespace Modules
{
	class LcgRandom
	{
	private:
		uint64_t x;
		uint64_t a;
		uint64_t c;

		float floatIntInv;

	public:
		inline LcgRandom(uint64_t seed = 0)
		{
			x = seed;
			a = 22695477;
			c = 1;

			floatIntInv = 1.0 / (float)INT32_MAX;
		}

		inline void SetSeed(uint64_t seed)
		{
			x = seed;
		}

		inline uint32_t NextUInt()
		{
			uint64_t axc = a * x + c;
			//x = axc % m;
			x = axc & 0xFFFFFFFF;
			return (uint32_t)x;
		}

		inline int32_t NextInt()
		{
			int64_t axc = a * x + c;
			//x = axc % m;
			x = axc & 0x7FFFFFFF;
			return (int32_t)x;
		}

		inline float NextFloat()
		{
			auto n = NextInt();
			return n * floatIntInv;
		}

		inline void GetFloats(float* buffer, int len)
		{
			for (int i = 0; i < len; i++)
				buffer[i] = NextFloat();
		}
	};
}
