#pragma once

#include <cmath>
#include <cstdint>
#include <cstring>
#include <limits>

#ifndef ATLAS_HAVE_HEX_FP_CONSTANTS
static const double DoubleOneMinusEpsilon = 0.99999999999999989;
static const float FloatOneMinusEpsilon = 0.99999994f;
#else
static const double DoubleOneMinusEpsilon = 0x1.fffffffffffffp-1;
static const float FloatOneMinusEpsilon = 0x1.fffffep-1;
#endif

#ifdef ATLAS_USE_DOUBLE_AS_FLOAT

#define PI 3.14159265358979323846264338327950288

typedef double Float;

#else

typedef float Float;

#endif

#ifdef _MSC_VER
#define MACHINE_EPSILON (std::numeric_limits<Float>::epsilon() * 0.5)
#else
static Float MACHINE_EPSILON = std::numeric_limits<Float>::epsilon() * 0.5;
#endif

static constexpr Float SHADOW_EPSILON = static_cast<Float>(0.0001);
static constexpr Float PI = static_cast<Float>(3.14159265358979323846);
static constexpr Float INV_PI = static_cast<Float>(0.31830988618379067154);
static constexpr Float INV_2PI = static_cast<Float>(0.15915494309189533577);
static constexpr Float INV_4PI = static_cast<Float>(0.07957747154594766788);
static constexpr Float PI_OVER2 = static_cast<Float>(1.57079632679489661923);
static constexpr Float PI_OVER4 = static_cast<Float>(0.78539816339744830961);
static constexpr Float SQRT2 = static_cast<Float>(1.41421356237309504880);

#ifdef ATLAS_FLOAT_AS_DOUBLE
static const Float OneMinusEpsilon = DoubleOneMinusEpsilon;
#else
static const Float OneMinusEpsilon = FloatOneMinusEpsilon;
#endif

#define NBR_RAY_PER_CONE 64

namespace atlas
{
	struct RgbSpectrum;
	typedef RgbSpectrum Spectrum;

	inline uint32_t floatToBits(float f)
	{
		uint32_t ui;
		memcpy(&ui, &f, sizeof(float));
		return ui;
	}

	inline float bitsToFloat(uint32_t ui)
	{
		float f;
		memcpy(&f, &ui, sizeof(uint32_t));
		return f;
	}

	inline uint64_t floatToBits(double f)
	{
		uint64_t ui;
		memcpy(&ui, &f, sizeof(double));
		return ui;
	}

	inline double bitsToFloat(uint64_t ui)
	{
		double f;
		memcpy(&f, &ui, sizeof(uint64_t));
		return f;
	}

	inline float nextFloatUp(float v)
	{
		if (std::isinf(v) && v > 0.)
			return v;
		if (v == -0.f)
			v = 0.f;

		uint32_t ui = floatToBits(v);
		if (v >= 0)
			++ui;
		else
			--ui;
		return bitsToFloat(ui);
	}

	inline float nextFloatDown(float v)
	{
		if (std::isinf(v) && v < 0.)
			return v;
		if (v == 0.f)
			v = -0.f;
		uint32_t ui = floatToBits(v);
		if (v > 0)
			--ui;
		else
			++ui;
		return bitsToFloat(ui);
	}

	inline void uintToBits(uint32_t src, uint32_t &dst, uint32_t size, uint32_t offset)
	{
		for (uint32_t i = 0; i < size; i++)
		{
			dst |= (((src >> i) & 1) << (offset + i));
		}
	}

	inline uint32_t bitsToUint(uint32_t src, uint32_t size, uint32_t offset)
	{
		uint32_t dst = 0;
		for (uint32_t i = 0; i < size; i++)
		{
			dst |= (((src >> (offset + i)) & 1) << i);
		}
		return (dst);
	}

	inline Float gamma(int n)
	{
		return ((Float)n * MACHINE_EPSILON) / (1. - (Float)n * MACHINE_EPSILON);
	}

	enum class TransportMode
	{
		Radiance,
		Importance
	};
}