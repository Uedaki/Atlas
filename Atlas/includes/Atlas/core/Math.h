#pragma once

#include <cmath>

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"

namespace atlas
{
	inline Float radians(Float v)
	{
		return (v * PI / (Float)180.0);
	}

	inline Float degrees(Float v)
	{
		return (static_cast<Float>(v * 180.0 / PI));
	}

	template <typename T>
	T lerp(T a, T b, Float t)
	{
		return (1.f - t) * a + t * b;
	}

	template <typename T, typename U>
	T clamp(T a, U low, U high)
	{
		return (a >= low ? (a <= high ? a : high) : low);
	}

	template <typename Ta, typename Tb, typename Tc, typename Td>
	inline auto differenceOfProducts(Ta a, Tb b, Tc c, Td d)
	{
		auto cd = c * d;
		auto diff = std::fma(a, b, -cd);
		auto error = std::fma(-c, d, cd);
		return (diff + error);
	}

	template <typename Ta, typename Tb, typename Tc, typename Td>
	inline auto sumOfProducts(Ta a, Tb b, Tc c, Td d)
	{
		auto cd = c * d;
		auto diff = std::fma(a, b, cd);
		auto error = std::fma(c, d, -cd);
		return (diff + error);
	}
}