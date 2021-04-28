#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ostream>

#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/Math.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	template <typename T>
	struct Point2
	{
		T x = 0;
		T y = 0;

		Point2() = default;
		Point2(T initialValue)
			: x(initialValue), y(initialValue)
		{
			//DCHECK(!hasNans());
		}
		Point2(T initialX, T initialY)
			: x(initialX), y(initialY)
		{
			//DCHECK(!hasNans());
		}

		//Point2(const Point3<T> &p)
		//	: x(p.x), y(p.y)
		//{
		//	DCHECK(!hasNans());
		//}

		template <typename U>
		explicit Point2(const Point2<U> &p)
			: x(static_cast<U>(p.x))
			, y(static_cast<U>(p.y))
		{
			//DCHECK(!hasNans());
		}

		template <typename U>
		explicit operator Vector2<U>() const
		{
			return (Vector2<U>(x, y));
		}

		bool hasNans() const
		{
			return (std::isnan(x) || std::isnan(y));
		}

		T minComponent()
		{
			return (std::min(x, y));
		}

		T maxComponent()
		{
			return (std::max(x, y));
		}

		int8_t maxDimension()
		{
			return (x > y ? 0 : 1);
		}

		Point2<T> operator+(const Vector2<T> &v) const
		{
			return (Point2<T>(x + v.x, y + v.y));
		}

		Point2<T> &operator+=(const Vector2<T> &v)
		{
			x += v.x;
			y += v.y;
			return (*this);
		}

		Point2<T> operator-() const
		{
			return (Point2<T>(-x, -y));
		}

		Vector2<T> operator-(const Point2<T> &v) const
		{
			return (Vector2<T>(x - v.x, y - v.y));
		}

		Point2<T> operator-(const Vector2<T> &v) const
		{
			return (Point2<T>(x - v.x, y - v.y));
		}

		Point2<T> &operator-=(const Vector2<T> &v)
		{
			x -= v.x;
			y -= v.y;
			return (*this);
		}

		Point2<T> operator+(const Point2<T> &v) const
		{
			return (Point2<T>(x + v.x, y + v.y));
		}

		Point2<T> &operator+=(const Point2<T> &v)
		{
			x += v.x;
			y += v.y;
			return (*this);
		}

		Point2<T> operator*(T scalar) const
		{
			return (Point2<T>(x * scalar, y * scalar));
		}

		Point2<T> &operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			return (*this);
		}

		Point2<T> operator/(T scalar) const
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			return (Point2<T>(x * inv, y * inv));
		}

		Point2<T> &operator/=(T scalar)
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			x *= inv;
			y *= inv;
			return (*this);
		}

		bool operator==(const Point2<T> &p) const
		{
			return (x == p.x && y == p.y);
		}

		bool operator!=(const Point2<T> &p) const
		{
			return (x != p.x || y != p.y);
		}

		T operator[](int i) const
		{
			DCHECK(i >= 0 && i <= 1);
			if (i == 0)
				return (x);
			else
				return (y);
		}

		T &operator[](int i)
		{
			DCHECK(i >= 0 && i <= 1);
			if (i == 0)
				return (x);
			else
				return (y);
		}

		friend std::ostream &operator<<(std::ostream &os, const Point2<T> &p)
		{
			os << "[" << p.x << ", " << p.y << "]";
			return (os);
		}
	};

	template <typename T>
	struct Point3
	{
		T x = 0;
		T y = 0;
		T z = 0;

		Point3() = default;
		Point3(T initialValue)
			: x(initialValue), y(initialValue), z(initialValue)
		{
			//DCHECK(hasNans());
		}
		Point3(T initialX, T initialY, T initialZ)
			: x(initialX), y(initialY), z(initialZ)
		{
			//DCHECK(hasNans());
		}

		template <typename U>
		explicit Point3(const Point2<U> &p)
			: x(static_cast<U>(p.x))
			, y(static_cast<U>(p.y))
			, z(static_cast<U>(p.z))
		{
			DCHECK(!hasNans());
		}

		template <typename U>
		explicit operator Vector3<U>() const
		{
			return (Vector3<U>(x, y, z));
		}

		T minComponent()
		{
			return (std::min(x, std::min(y, z)));
		}

		T maxComponent()
		{
			return (std::max(x, std::max(y, z)));
		}

		int8_t maxDimension()
		{
			return (x > y ? (x > z ? 0 : 2) : (y > z ? 1 : 2));
		}

		bool hasNans() const
		{
			return (std::isnan(x) || std::isnan(y) || std::isnan(z));
		}

		Point3<T> operator+(const Vector3<T> &v) const
		{
			return (Point3<T>(x + v.x, y + v.y, z + v.z));
		}

		Point3<T> &operator+=(const Vector3<T> &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return (*this);
		}

		Point3<T> operator-() const
		{
			return (Point3<T>(-x, -y, -z));
		}

		Vector3<T> operator-(const Point3<T> &v) const
		{
			return (Vector3<T>(x - v.x, y - v.y, z - v.z));
		}

		Point3<T> operator-(const Vector3<T> &v) const
		{
			return (Point3<T>(x - v.x, y - v.y, z - v.z));
		}

		Point3<T> &operator-=(const Vector3<T> &v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return (*this);
		}

		Point3<T> operator+(const Point3<T> &v) const
		{
			return (Point3<T>(x + v.x, y + v.y, z + v.z));
		}

		Point3<T> &operator+=(const Point3<T> &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return (*this);
		}

		Point3<T> operator*(T scalar) const
		{
			return (Point3<T>(x * scalar, y * scalar, z * scalar));
		}

		Point3<T> &operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return (*this);
		}

		Point3<T> operator/(T scalar) const
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			return (Point3<T>(x * inv, y * inv, z * inv));
		}

		Point3<T> &operator/=(T scalar)
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			x *= inv;
			y *= inv;
			z *= inv;
			return (*this);
		}

		bool operator==(const Point3<T> &p) const
		{
			return (x == p.x && y == p.y && z == p.z);
		}

		bool operator!=(const Point3<T> &p) const
		{
			return (x != p.x || y != p.y || z != p.z);
		}

		T operator[](int i) const
		{
			DCHECK(i >= 0 && i <= 3);
			if (i == 0)
				return (x);
			else if (i == 1)
				return (y);
			return (z);
		}

		T &operator[](int i)
		{
			DCHECK(i >= 0 && i <= 3);
			if (i == 0)
				return (x);
			else if (i == 1)
				return (y);
			return (z);
		}

		friend std::ostream &operator<<(std::ostream &os, const Point3<T> &p)
		{
			os << "[" << p.x << ", " << p.y << ", " << p.z << "]";
			return (os);
		}
	};

	template <typename T>
	inline Point2<T> operator*(T scalar, const Point2<T> &p)
	{
		return (p * scalar);
	}

	template <typename T>
	inline Point3<T> operator*(T scalar, const Point3<T> &p)
	{
		return (p * scalar);
	}

	template <typename T>
	inline Float distance(const Point2<T> &p1, const Point2<T> &p2)
	{
		return ((p1 - p2).length());
	}

	template <typename T>
	inline Float distance(const Point3<T> &p1, const Point3<T> &p2)
	{
		return ((p1 - p2).length());
	}

	template <typename T>
	inline Float distanceSquared(const Point2<T> &p1, const Point2<T> &p2)
	{
		return ((p1 - p2).lengthSquared());
	}

	template <typename T>
	inline Float distanceSquared(const Point3<T> &p1, const Point3<T> &p2)
	{
		return ((p1 - p2).lengthSquared());
	}

	template <typename T>
	Point2<T> lerp(const Point2<T> &p1, const Point2<T> &p2, Float t)
	{
		return (1.f - t) * p1 + t * p2;
	}

	template <typename T>
	Point3<T> lerp(const Point3<T> &p1, const Point3<T> &p2, Float t)
	{
		return (1.f - t) * p1 + t * p2;
	}

	template <typename T>
	Point2<T> min(const Point2<T> &v1, const Point2<T> &v2)
	{
		return (Point2<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y)));
	}

	template <typename T>
	Point3<T> min(const Point3<T> &v1, const Point3<T> &v2)
	{
		return (Point3<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z)));
	}

	template <typename T>
	Point2<T> max(const Point2<T> &v1, const Point2<T> &v2)
	{
		return (Point2<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y)));
	}

	template <typename T>
	Point3<T> max(const Point3<T> &v1, const Point3<T> &v2)
	{
		return (Point3<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z)));
	}

	template <typename T>
	Point2<T> floor(const Point2<T> &p)
	{
		return (Point2<T>(std::floor(p.x), std::floor(p.y)));
	}

	template <typename T>
	Point3<T> floor(const Point3<T> &p)
	{
		return (Point3<T>(std::floor(p.x), std::floor(p.y), std::floor(p.z)));
	}

	template <typename T>
	Point2<T> ceil(const Point2<T> &p)
	{
		return (Point2<T>(std::ceil(p.x), std::ceil(p.y)));
	}

	template <typename T>
	Point3<T> ceil(const Point3<T> &p)
	{
		return (Vector3<T>(std::ceil(p.x), std::ceil(p.y), std::ceil(p.z)));
	}

	template <typename T>
	Point2<T> abs(const Point2<T> &p)
	{
		return (Point2<T>(std::abs(p.x), std::abs(p.y)));
	}

	template <typename T>
	Point3<T> abs(const Point3<T> &p)
	{
		return (Point3<T>(std::abs(p.x), std::abs(p.y), std::abs(p.z)));
	}

	template <typename T>
	Point2<T> permute(const Point2<T> &v, int x, int y) {
		return Point2<T>(v[x], v[y]);
	}

	template <typename T>
	Point3<T> permute(const Point3<T> &v, int x, int y, int z) {
		return Point3<T>(v[x], v[y], v[z]);
	}

	typedef Point2<Float> Point2f;
	typedef Point2<int32_t> Point2i;
	typedef Point3<Float> Point3f;
	typedef Point3<int32_t> Point3i;
}