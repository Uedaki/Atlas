#pragma once

#ifdef _USE_SIMD

#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/simd/Simd.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	struct S4Vector3
	{
		S4Float x;
		S4Float y;
		S4Float z;

		SIMD_INLINE S4Vector3() = default;
		SIMD_INLINE S4Vector3(Float s)
			: x(s), y(s), z(s)
		{
			DCHECK(std::isnan(s));
		}
		SIMD_INLINE S4Vector3(const Vec3 &v1)
			: x(v1.x, v1.x, v1.x, v1.x)
			, y(v1.y, v1.y, v1.y, v1.y)
			, z(v1.z, v1.z, v1.z, v1.z)
		{
			DCHECK(!v1.hasNans());
		}
		SIMD_INLINE S4Vector3(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3, const Vec3 &v4)
			: x(v1.x, v2.x, v3.x, v4.x)
			, y(v1.y, v2.y, v3.y, v4.y)
			, z(v1.z, v2.z, v3.z, v4.z)
		{
			DCHECK(!v1.hasNans() && !v2.hasNans() && !v3.hasNans() && v4.hasNans());
		}
		SIMD_INLINE S4Vector3(const S4Float &f1, const S4Float &f2, const S4Float &f3)
			: x(f1)
			, y(f2)
			, z(f3)
		{
		}

		SIMD_INLINE S4Float lengthSquared() const
		{
			return (x * x + y * y + z * z);
		}

		SIMD_INLINE S4Float length() const
		{
			return (S4Float(sqrtf(lengthSquared())));
		}

		SIMD_INLINE S4Vector3 operator+(const S4Vector3 &v) const
		{
			return (S4Vector3(x + v.x, y + v.y, z + v.z));
		}

		SIMD_INLINE S4Vector3 &operator+=(const S4Vector3 &v)
		{
			x = x + v.x;
			y = y + v.y;
			z = z + v.z;
			return (*this);
		}

		SIMD_INLINE S4Vector3 operator-() const
		{
			return (S4Vector3(-x, -y, -z));
		}

		SIMD_INLINE S4Vector3 operator-(const S4Vector3 &v) const
		{
			return (S4Vector3(x - v.x, y - v.y, z - v.z));
		}

		SIMD_INLINE S4Vector3 &operator-=(const S4Vector3 &v)
		{
			x = x - v.x;
			y = y - v.y;
			z = z - v.z;
			return (*this);
		}

		SIMD_INLINE S4Vector3 operator*(S4Float scalar) const
		{
			return (S4Vector3(x * scalar, y * scalar, z * scalar));
		}

		SIMD_INLINE S4Vector3 &operator*=(S4Float scalar)
		{
			x = x * scalar;
			y = y * scalar;
			z = z * scalar;
			return (*this);
		}

		SIMD_INLINE S4Vector3 operator/(S4Float scalar) const
		{
			S4Float inv = S4Float(1) / scalar;
			return (S4Vector3(x * inv, y * inv, z * inv));
		}

		SIMD_INLINE S4Vector3 &operator/=(S4Float scalar)
		{
			S4Float inv = S4Float(1) / scalar;
			x = x * inv;
			y = y * inv;
			z = z * inv;
			return (*this);
		}

		SIMD_INLINE S4Float operator[](int8_t i) const
		{
			DCHECK(i >= 0 && i <= 3);
			if (i == 0)
				return (x);
			else if (i == 1)
				return (y);
			return (z);
		}

		SIMD_INLINE S4Float &operator[](int8_t i)
		{
			DCHECK(i >= 0 && i <= 3);
			if (i == 0)
				return (x);
			else if (i == 1)
				return (y);
			return (z);
		}
	};

	SIMD_INLINE S4Float dot(const S4Vector3 &v1, const S4Vector3 &v2)
	{
		return (v1.x * v2.x + v1.y * v2.y);
	}

	SIMD_INLINE S4Vector3 cross(const S4Vector3 &v1, const S4Vector3 &v2)
	{
		return (S4Vector3(v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x));
	}

	SIMD_INLINE S4Vector3 normalize(const S4Vector3 &v)
	{
		return (v / v.length());
	}

	SIMD_INLINE S4Vector3 min(const S4Vector3 &v1, const S4Vector3 &v2)
	{
		return (S4Vector3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z)));
	}

	SIMD_INLINE S4Vector3 max(const S4Vector3 &v1, const S4Vector3 &v2)
	{
		return (S4Vector3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z)));
	}

	SIMD_INLINE S4Vector3 operator*(S4Float scalar, const S4Vector3 &v)
	{
		return (v * scalar);
	}

	SIMD_INLINE S4Vector3 select(S4Vector3 a, S4Vector3 b, S4Bool cond)
	{
		return (S4Vector3(select(a.x, b.x, cond), select(a.y, b.y, cond),
			select(a.z, b.z, cond)));
	}

	typedef S4Vector3 S4Vec3;
	typedef S4Vector3 S4Normal;
}

#endif