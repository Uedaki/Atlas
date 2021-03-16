#pragma once

#ifdef _USE_SIMD

#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/Points.h"
#include "atlas/core/simd/SVectors.h"
#include "atlas/core/simd/Simd.h"

namespace atlas
{
	struct S4Point2
	{
		S4Float x;
		S4Float y;

		SIMD_INLINE S4Point2() = default;
		SIMD_INLINE S4Point2(Float s)
			: x(s), y(s)
		{
			DCHECK(!std::isnan(s));
		}
		SIMD_INLINE S4Point2(const Point2f &v1)
			: x(v1.x), y(v1.y)
		{
			DCHECK(!v1.hasNans());
		}
		SIMD_INLINE S4Point2(const Point2f &p1, const Point2f &p2, const Point2f &p3, const Point2f &p4)
			: x(p1.x, p2.x, p2.x, p4.x)
			, y(p1.y, p2.y, p3.y, p4.y)
		{
			DCHECK(!p1.hasNans() && !p2.hasNans() && !p3.hasNans() && !p4.hasNans());
		}
		SIMD_INLINE S4Point2(const S4Float &f1, const S4Float &f2)
			: x(f1), y(f2)
		{}

		//SIMD_INLINE S4Point2 operator+(const S4Vec4 &v) const
		//{
		//	return (S4Point3(x + v.x, y + v.y, z + v.z));
		//}

		//SIMD_INLINE S4Point3 &operator+=(const S4Vec3 &v)
		//{
		//	x = x + v.x;
		//	y = y + v.y;
		//	z = z + v.z;
		//	return (*this);
		//}

		SIMD_INLINE S4Point2 operator-() const
		{
			return (S4Point2(-x, -y));
		}

		//SIMD_INLINE S4Vec3 operator-(const S4Point3 &v) const
		//{
		//	return (S4Vec3(x - v.x, y - v.y, z - v.z));
		//}

		//SIMD_INLINE S4Point3 operator-(const S4Vec3 &v) const
		//{
		//	return (S4Point3(x - v.x, y - v.y, z - v.z));
		//}

		//SIMD_INLINE S4Point3 &operator-=(const S4Vec3 &v)
		//{
		//	x = x - v.x;
		//	y = y - v.y;
		//	z = z - v.z;
		//	return (*this);
		//}

		SIMD_INLINE S4Point2 operator+(const S4Point2 &v) const
		{
			return (S4Point2(x + v.x, y + v.y));
		}

		SIMD_INLINE S4Point2 &operator+=(const S4Point2 &v)
		{
			x = x + v.x;
			y = y + v.y;
			return (*this);
		}

		SIMD_INLINE S4Point2 operator*(S4Float scalar) const
		{
			return (S4Point2(x * scalar, y * scalar));
		}

		SIMD_INLINE S4Point2 &operator*=(S4Float scalar)
		{
			x = x * scalar;
			y = y * scalar;
			return (*this);
		}

		SIMD_INLINE S4Point2 operator/(S4Float scalar) const
		{
			S4Float inv = S4Float(1) / scalar;
			return (S4Point2(x * inv, y * inv));
		}

		SIMD_INLINE S4Point2 &operator/=(S4Float scalar)
		{
			S4Float inv = S4Float(1) / scalar;
			x = x * inv;
			y = y * inv;
			return (*this);
		}

		SIMD_INLINE S4Bool operator==(const S4Point2 &p) const
		{
			return ((x == p.x) & (y == p.y));
		}

		SIMD_INLINE S4Bool operator!=(const S4Point2 &p) const
		{
			return ((x != p.x) | (y != p.y));
		}

		SIMD_INLINE S4Float operator[](int i) const
		{
			DCHECK(i >= 0 && i <= 2);
			if (i == 0)
				return (x);
			return (y);
		}

		SIMD_INLINE S4Float &operator[](int i)
		{
			DCHECK(i >= 0 && i <= 2);
			if (i == 0)
				return (x);
			return (y);
		}
	};

	struct S4Point3
	{
		S4Float x;
		S4Float y;
		S4Float z;

		SIMD_INLINE S4Point3() = default;
		SIMD_INLINE S4Point3(Float s)
			: x(s), y(s), z(s)
		{
			DCHECK(!std::isnan(s));
		}
		SIMD_INLINE S4Point3(const Point3f &v1)
			: x(v1.x), y(v1.y), z(v1.z)
		{
			DCHECK(!v1.hasNans());
		}
		SIMD_INLINE S4Point3(const Point3f &p1, const Point3f &p2, const Point3f &p3, const Point3f &p4)
			: x(p1.x, p2.x, p2.x, p4.x)
			, y(p1.y, p2.y, p3.y, p4.y)
			, z(p1.z, p2.z, p3.z, p4.z)
		{
			DCHECK(!p1.hasNans() && !p2.hasNans() && !p3.hasNans() && !p4.hasNans());
		}
		SIMD_INLINE S4Point3(const S4Float &f1, const S4Float &f2, const S4Float &f3)
			: x(f1), y(f2), z(f3)
		{}

		SIMD_INLINE S4Point3 operator+(const S4Vec3 &v) const
		{
			return (S4Point3(x + v.x, y + v.y, z + v.z));
		}

		SIMD_INLINE S4Point3 &operator+=(const S4Vec3 &v)
		{
			x = x + v.x;
			y = y + v.y;
			z = z + v.z;
			return (*this);
		}

		SIMD_INLINE S4Point3 operator-() const
		{
			return (S4Point3(-x, -y, -z));
		}

		SIMD_INLINE S4Vec3 operator-(const S4Point3 &v) const
		{
			return (S4Vec3(x - v.x, y - v.y, z - v.z));
		}

		SIMD_INLINE S4Point3 operator-(const S4Vec3 &v) const
		{
			return (S4Point3(x - v.x, y - v.y, z - v.z));
		}

		SIMD_INLINE S4Point3 &operator-=(const S4Vec3 &v)
		{
			x = x - v.x;
			y = y - v.y;
			z = z - v.z;
			return (*this);
		}

		SIMD_INLINE S4Point3 operator+(const S4Point3 &v) const
		{
			return (S4Point3(x + v.x, y + v.y, z + v.z));
		}

		SIMD_INLINE S4Point3 &operator+=(const S4Point3 &v)
		{
			x = x + v.x;
			y = y + v.y;
			z = z + v.z;
			return (*this);
		}

		SIMD_INLINE S4Point3 operator*(S4Float scalar) const
		{
			return (S4Point3(x * scalar, y * scalar, z * scalar));
		}

		SIMD_INLINE S4Point3 &operator*=(S4Float scalar)
		{
			x = x * scalar;
			y = y * scalar;
			z = z * scalar;
			return (*this);
		}

		SIMD_INLINE S4Point3 operator/(S4Float scalar) const
		{
			S4Float inv = S4Float(1) / scalar;
			return (S4Point3(x * inv, y * inv, z * inv));
		}

		SIMD_INLINE S4Point3 &operator/=(S4Float scalar)
		{
			S4Float inv = S4Float(1) / scalar;
			x = x * inv;
			y = y * inv;
			z = z * inv;
			return (*this);
		}

		SIMD_INLINE S4Bool operator==(const S4Point3 &p) const
		{
			return ((x == p.x) & (y == p.y) & (z == p.z));
		}

		SIMD_INLINE S4Bool operator!=(const S4Point3 &p) const
		{
			return ((x != p.x) | (y != p.y) | (z != p.z));
		}

		SIMD_INLINE S4Float operator[](int i) const
		{
			DCHECK(i >= 0 && i <= 3);
			if (i == 0)
				return (x);
			else if (i == 1)
				return (y);
			return (z);
		}

		SIMD_INLINE S4Float &operator[](int i)
		{
			DCHECK(i >= 0 && i <= 3);
			if (i == 0)
				return (x);
			else if (i == 1)
				return (y);
			return (z);
		}
	};

	SIMD_INLINE S4Float distance(const S4Point3 &p1, const S4Point3 &p2)
	{
		return ((p1 - p2).length());
	}

	SIMD_INLINE S4Float distanceSquared(const S4Point3 &p1, const S4Point3 &p2)
	{
		return ((p1 - p2).lengthSquared());
	}

	SIMD_INLINE S4Point3 lerp(const S4Point3 &p1, const S4Point3 &p2, S4Float t)
	{
		return (p1 * (S4Float(1.f) - t) + p2 * t);
	}

	SIMD_INLINE S4Point3 min(const S4Point3 &v1, const S4Point3 &v2)
	{
		return (S4Point3(min(v1.x, v2.x), min(v1.y, v2.y), min(v1.z, v2.z)));
	}

	SIMD_INLINE S4Point3 max(const S4Point3 &v1, const S4Point3 &v2)
	{
		return (S4Point3(max(v1.x, v2.x), max(v1.y, v2.y), max(v1.z, v2.z)));
	}

	SIMD_INLINE S4Point3 select(S4Point3 a, S4Point3 b, S4Bool cond)
	{
		return (S4Point3(select(a.x, b.x, cond), select(a.y, b.y, cond),
			select(a.z, b.z, cond)));
	}
}

#endif