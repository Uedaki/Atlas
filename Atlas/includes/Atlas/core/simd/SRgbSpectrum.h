#pragma once

#include "Atlas/Atlas.h"
#include "Atlas/core/Simd/Simd.h"

namespace atlas
{
	struct S4RgbSpectrum
	{
		S4Float r;
		S4Float g;
		S4Float b;

		SIMD_INLINE S4RgbSpectrum() = default;
		SIMD_INLINE S4RgbSpectrum(Float v)
			: r(v), g(v), b(v)
		{}
		SIMD_INLINE S4RgbSpectrum(Float r, Float g, Float b)
			: r(r), g(g), b(b)
		{}
		SIMD_INLINE S4RgbSpectrum(Spectrum a, Spectrum b, Spectrum c, Spectrum d)
			: r(a.r, b.r, c.r, d.r), g(a.g, b.g, c.g, d.g), b(a.b, b.b, c.b, d.b)
		{}
		SIMD_INLINE S4RgbSpectrum(S4Float r, S4Float g, S4Float b)
			: r(r), g(g), b(b)
		{}

		SIMD_INLINE S4RgbSpectrum operator+(const S4RgbSpectrum &spectrum) const
		{
			return (S4RgbSpectrum(r + spectrum.r, g + spectrum.g, b + spectrum.b));
		}

		SIMD_INLINE S4RgbSpectrum &operator+=(const S4RgbSpectrum &spectrum)
		{
			r = r + spectrum.r;
			g = g + spectrum.g;
			b = b + spectrum.b;
			return (*this);
		}

		SIMD_INLINE S4RgbSpectrum operator-(const S4RgbSpectrum &spectrum) const
		{
			return (S4RgbSpectrum(r - spectrum.r, g - spectrum.g, b - spectrum.b));
		}

		S4RgbSpectrum &operator-=(const S4RgbSpectrum &spectrum)
		{
			r = r + spectrum.r;
			g = g + spectrum.g;
			b = b + spectrum.b;
			return (*this);
		}

		SIMD_INLINE S4RgbSpectrum operator*(const S4RgbSpectrum &spectrum) const
		{
			return (S4RgbSpectrum(r * spectrum.r, g * spectrum.g, b * spectrum.b));
		}

		SIMD_INLINE S4RgbSpectrum &operator*=(const S4RgbSpectrum &spectrum)
		{
			r = r * spectrum.r;
			g = g * spectrum.g;
			b = b * spectrum.b;
			return (*this);
		}

		SIMD_INLINE S4RgbSpectrum operator*(S4Float scalar) const
		{
			return (S4RgbSpectrum(r * scalar, g * scalar, b * scalar));
		}

		SIMD_INLINE S4RgbSpectrum &operator*=(S4Float scalar)
		{
			r = r * scalar;
			g = g * scalar;
			b = b * scalar;
			return (*this);
		}

		SIMD_INLINE S4RgbSpectrum operator/(const S4RgbSpectrum &spectrum) const
		{
			return (S4RgbSpectrum(r / spectrum.r, g / spectrum.g, b / spectrum.b));
		}

		SIMD_INLINE S4RgbSpectrum &operator/=(const S4RgbSpectrum &spectrum)
		{
			r = r / spectrum.r;
			g = g / spectrum.g;
			b = b / spectrum.b;
			return (*this);
		}

		SIMD_INLINE S4RgbSpectrum operator/(S4Float scalar) const
		{
			S4Float inv = S4Float(1, 1, 1, 1) / scalar;
			return (S4RgbSpectrum(r * inv, g * inv, b * inv));
		}

		SIMD_INLINE S4RgbSpectrum &operator/=(S4Float scalar)
		{
			S4Float inv = S4Float(1, 1, 1, 1) / scalar;
			r = r * scalar;
			g = g * scalar;
			b = b * scalar;
			return (*this);
		}

		SIMD_INLINE S4Bool operator==(const S4RgbSpectrum &s) const
		{
			return (r == s.r & g == s.g & b == s.b);
		}

		SIMD_INLINE S4Bool operator!=(const S4RgbSpectrum &s) const
		{
			return (r != s.r | g != s.g | b != s.b);
		}

		SIMD_INLINE S4Bool isBlack() const
		{
			return (r == S4Float(0.f, 0.f, 0.f, 0.f) & g == S4Float(0.f, 0.f, 0.f, 0.f) & b == S4Float(0.f, 0.f, 0.f, 0.f));
		}

		SIMD_INLINE S4Bool isWhite() const
		{
			return (r == S4Float(1.f, 1.f, 1.f, 1.f) & g == S4Float(1.f, 1.f, 1.f, 1.f) & b == S4Float(1.f, 1.f, 1.f, 1.f));
		}

		SIMD_INLINE S4RgbSpectrum getClampedSpectrum(S4Float low = S4Float(0.f, 0.f, 0.f, 0.f), S4Float high = S4Float(1.f, 1.f, 1.f, 1.f)) const
		{
			return (S4RgbSpectrum(clamp(r, low, high), clamp(r, low, high), clamp(r, low, high)));
		}

		SIMD_INLINE S4Float y() const
		{
			const S4Float YWeight[3] = { S4Float(0.212671f), S4Float(0.715160f), S4Float(0.072169f) };
			return YWeight[0] * r + YWeight[1] * g + YWeight[2] * b;
		}

		SIMD_INLINE S4Float &operator[](int i)
		{
			if (i == 0)
				return (r);
			else if (i == 1)
				return (g);
			return (b);
		}

		SIMD_INLINE S4Float operator[](int i) const
		{
			if (i == 0)
				return (r);
			else if (i == 1)
				return (g);
			return (b);
		}
	};

	SIMD_INLINE S4RgbSpectrum operator+(S4Float scalar, const S4RgbSpectrum &s)
	{
		return (S4RgbSpectrum(s.r + scalar, s.g + scalar, s.b + scalar));
	}

	SIMD_INLINE S4RgbSpectrum operator-(S4Float scalar, const S4RgbSpectrum &s)
	{
		return (S4RgbSpectrum(s.r * S4Float(-1) + scalar, s.g * S4Float(-1) + scalar, s.b * S4Float(-1) + scalar));
	}

	SIMD_INLINE S4RgbSpectrum operator*(S4Float scalar, const S4RgbSpectrum &s)
	{
		return (s * scalar);
	}

	SIMD_INLINE S4RgbSpectrum sqrt(const S4RgbSpectrum &s)
	{
		return (S4RgbSpectrum(sqrtf(s.r), sqrtf(s.g), sqrtf(s.b)));
	}

	SIMD_INLINE S4RgbSpectrum select(S4RgbSpectrum a, S4RgbSpectrum b, S4Bool mask)
	{
		return (S4RgbSpectrum(select(a.r, b.r, mask), select(a.g, b.g, mask), select(a.b, b.b, mask)));
	}

	SIMD_INLINE void swap(S4RgbSpectrum &a, S4RgbSpectrum &b, S4Bool mask)
	{
		S4RgbSpectrum tmp = a;
		a = select(a, b, mask);
		b = select(b, a, mask == S4Bool(0, 0, 0, 0));
	}
}