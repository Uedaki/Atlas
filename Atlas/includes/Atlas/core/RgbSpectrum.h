#pragma once

#include <algorithm>
#include <cmath>
#include <ostream>

#include "atlas/Atlas.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Math.h"

namespace atlas
{
	struct RgbSpectrum
	{
		Float r;
		Float g;
		Float b;

		RgbSpectrum() = default;
		constexpr RgbSpectrum(Float v)
			: r(v), g(v), b(v)
		{}
		constexpr RgbSpectrum(Float r, Float g, Float b)
			: r(r), g(g), b(b)
		{}

		RgbSpectrum operator+(const RgbSpectrum &spectrum) const
		{
			return (RgbSpectrum(r + spectrum.r, g + spectrum.g, b + spectrum.b));
		}

		RgbSpectrum &operator+=(const RgbSpectrum &spectrum)
		{
			r += spectrum.r;
			g += spectrum.g;
			b += spectrum.b;
			return (*this);
		}

		RgbSpectrum operator-(const RgbSpectrum &spectrum) const
		{
			return (RgbSpectrum(r - spectrum.r, g - spectrum.g, b - spectrum.b));
		}

		RgbSpectrum &operator-=(const RgbSpectrum &spectrum)
		{
			r -= spectrum.r;
			g -= spectrum.g;
			b -= spectrum.b;
			return (*this);
		}

		RgbSpectrum operator*(const RgbSpectrum &spectrum) const
		{
			return (RgbSpectrum(r * spectrum.r, g * spectrum.g, b * spectrum.b));
		}

		RgbSpectrum &operator*=(const RgbSpectrum &spectrum)
		{
			r *= spectrum.r;
			g *= spectrum.g;
			b *= spectrum.b;
			return (*this);
		}

		RgbSpectrum operator*(Float scalar) const
		{
			return (RgbSpectrum(r * scalar, g * scalar, b * scalar));
		}

		RgbSpectrum &operator*=(Float scalar)
		{
			r *= scalar;
			g *= scalar;
			b *= scalar;
			return (*this);
		}

		RgbSpectrum operator/(const RgbSpectrum &spectrum) const
		{
			return (RgbSpectrum(r / spectrum.r, g / spectrum.g, b / spectrum.b));
		}

		RgbSpectrum &operator/=(const RgbSpectrum &spectrum)
		{
			r /= spectrum.r;
			g /= spectrum.g;
			b /= spectrum.b;
			return (*this);
		}

		RgbSpectrum operator/(Float scalar) const
		{
			Float inv = 1 / scalar;
			return (RgbSpectrum(r * inv, g * inv, b * inv));
		}

		RgbSpectrum &operator/=(Float scalar)
		{
			Float inv = 1 / scalar;
			r *= scalar;
			g *= scalar;
			b *= scalar;
			return (*this);
		}

		bool operator==(const RgbSpectrum &s) const
		{
			return (r == s.r && g == s.g && b == s.b);
		}

		bool operator!=(const RgbSpectrum &s) const
		{
			return (r != s.r || g != s.g || b != s.b);
		}

		bool isBlack() const
		{
			return (r == 0.f && g == 0.f && b == 0.f);
		}

		bool isWhite() const
		{
			return (r == 1.f && g == 1.f && b == 1.f);
		}

		RgbSpectrum getClampedSpectrum(Float low = 0., Float high = 1.) const
		{
			return (RgbSpectrum(clamp(r, low, high), clamp(r, low, high), clamp(r, low, high)));
		}

		Float y() const
		{
			const Float YWeight[3] = { 0.212671f, 0.715160f, 0.072169f };
			return YWeight[0] * r + YWeight[1] * g + YWeight[2] * b;
		}

		Float &operator[](int i)
		{
			if (i == 0)
				return (r);
			else if (i == 1)
				return (g);
			return (b);
		}

		Float operator[](int i) const
		{
			if (i == 0)
				return (r);
			else if (i == 1)
				return (g);
			return (b);
		}

		bool hasNans() const
		{
			return (std::isnan(r) && std::isnan(g) && std::isnan(b));
		}
	};

	static constexpr RgbSpectrum BLACK = RgbSpectrum(0);
	static constexpr RgbSpectrum WHITE = RgbSpectrum(1);
	static constexpr RgbSpectrum RED = RgbSpectrum(1, 0, 0);
	static constexpr RgbSpectrum GREEN = RgbSpectrum(0, 1, 0);
	static constexpr RgbSpectrum BLUE = RgbSpectrum(0, 0, 1);

	inline RgbSpectrum operator+(Float scalar, const RgbSpectrum &s)
	{
		return (s + scalar);
	}

	inline RgbSpectrum operator-(Float scalar, const RgbSpectrum &s)
	{
		return (s * -1 + scalar);
	}

	inline RgbSpectrum operator*(Float scalar, const RgbSpectrum &s)
	{
		return (s * scalar);
	}

	inline RgbSpectrum sqrt(const RgbSpectrum &s)
	{
		return (RgbSpectrum(std::sqrt(s.r), std::sqrt(s.g), std::sqrt(s.b)));
	}

	inline RgbSpectrum sqrt(const RgbSpectrum &s, Float e)
	{
		return (RgbSpectrum(std::pow(s.r, e), std::pow(s.g, e), std::pow(s.b, e)));
	}

	inline RgbSpectrum exp(const RgbSpectrum &s)
	{
		return (RgbSpectrum(std::exp(s.r), std::exp(s.g), std::exp(s.b)));
	}

	template <uint32_t exp>
	constexpr inline float constPow(float v)
	{
		return (v * constPow<exp - 1>(v));
	}

	template <>
	constexpr inline float constPow<0>(float)
	{
		return (1);
	}

	inline uint32_t toRgb9e5(const RgbSpectrum &weight)
	{
		uint32_t cw = 0;

		constexpr uint32_t N = 9; // N is the number of mantissa bits per component
		constexpr uint32_t Emax = 31; // Emax is the maximum allowed biased exponent value
		constexpr uint32_t B = 15; // B is the exponent bias
		constexpr float sharedexp_max = (constPow<N>(2.f) - 1.f) / constPow<N>(2.f) * constPow<Emax - B>(2.f);

		const float red_c = std::max(0.f, std::min(sharedexp_max, weight.r));
		const float green_c = std::max(0.f, std::min(sharedexp_max, weight.g));
		const float blue_c = std::max(0.f, std::min(sharedexp_max, weight.b));

		const float max_c = std::max(std::max(red_c, green_c), blue_c);

		float exp_shared_p = std::max(-(float)B - 1, std::floor(log2(max_c))) + 1 + (float)B;

		const float max_s = std::floor(max_c / pow(2.f, (exp_shared_p - B - N)) + 0.5f);
		const float exp_shared = max_s == pow(2.f, N) ? exp_shared_p + 1.f : exp_shared_p;

		const float red_s = std::floor(red_c / pow(2.f, (exp_shared - B - N)) + 0.5f);
		const float green_s = std::floor(green_c / pow(2.f, (exp_shared - B - N)) + 0.5f);
		const float blue_s = std::floor(blue_c / pow(2.f, (exp_shared - B - N)) + 0.5f);

		uintToBits(static_cast<uint32_t>(red_s), cw, 9, 0);
		uintToBits(static_cast<uint32_t>(green_s), cw, 9, 9);
		uintToBits(static_cast<uint32_t>(blue_s), cw, 9, 18);
		uintToBits(static_cast<uint32_t>(exp_shared), cw, 5, 27);

		return (cw);
	}

	inline RgbSpectrum toColor(uint32_t weight)
	{
		RgbSpectrum color;

		const float N = 9; // N is the number of mantissa bits per component
		const float B = 15; // B is the exponent bias

		const float red_s = static_cast<float>(bitsToUint(weight, 9, 0));
		const float green_s = static_cast<float>(bitsToUint(weight, 9, 9));
		const float blue_s = static_cast<float>(bitsToUint(weight, 9, 18));
		const float exp_shared = static_cast<float>(bitsToUint(weight, 5, 27));

		color.r = red_s * pow(2.f, exp_shared - B - N);
		color.g = green_s * pow(2.f, exp_shared - B - N);
		color.b = blue_s * pow(2.f, exp_shared - B - N);

		return (color);
	}

	inline Float luminance(const Spectrum &color)
	{
		return (color.r * (Float)0.299 + color.g * (Float)0.587 + color.b * (Float)0.114);
	}

	inline std::ostream &operator<<(std::ostream &os, const RgbSpectrum &v)
	{
		os << "{" << v.r << ", " << v.g << ", " << v.b << "}";
		return (os);
	}
}