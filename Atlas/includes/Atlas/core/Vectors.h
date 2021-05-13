#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <ostream>

#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/Math.h"

namespace atlas
{
	template <typename T>
	struct Vector2
	{
		T x = 0;
		T y = 0;

		Vector2() = default;
		Vector2(T initialValue)
			: x(initialValue), y(initialValue)
		{
			//DCHECK(hasNans());
		}
		Vector2(T initialX, T initialY)
			: x(initialX), y(initialY)
		{
			//DCHECK(hasNans());
		}

		Float lengthSquared() const
		{
			return (x * x + y * y);
		}

		Float length() const
		{
			return (std::sqrt(lengthSquared()));
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

		Vector2<T> operator+(const Vector2<T> &v) const
		{
			return (Vector2<T>(x + v.x, y + v.y));
		}

		Vector2<T> &operator+=(const Vector2<T> &v)
		{
			x += v.x;
			y += v.y;
			return (*this);
		}

		Vector2<T> operator-() const
		{
			return (Vector2<T>(-x, -y));
		}

		Vector2<T> operator-(const Vector2<T> &v) const
		{
			return (Vector2<T>(x - v.x, y - v.y));
		}

		Vector2<T> &operator-=(const Vector2<T> &v)
		{
			x -= v.x;
			y -= v.y;
			return (*this);
		}

		Vector2<T> operator*(T scalar) const
		{
			return (Vector2<T>(x * scalar, y * scalar));
		}

		Vector2<T> &operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			return (*this);
		}

		Vector2<T> operator/(T scalar) const
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			return (Vector2<T>(x * inv, y * inv));
		}

		Vector2<T> &operator/=(T scalar)
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			x *= inv;
			y *= inv;
			return (*this);
		}

		bool operator==(const Vector2<T> &v) const
		{
			return (x == v.x && y == v.y);
		}

		bool operator!=(const Vector2<T> &v) const
		{
			return (!(*this == v));
		}

		T operator[](int i) const
		{
			DCHECK(i >= 0 && i <= 2);
			T values[2] = { x, y };
			return (values[i]);
		}

		T &operator[](int i)
		{
			DCHECK(i >= 0 && i <= 2);
			if (i == 0)
				return (x);
			else
				return (y);
		}
	};

	template <typename T>
	struct Vector3
	{
		T x = 0;
		T y = 0;
		T z = 0;

		Vector3() = default;
		Vector3(T initialValue)
			: x(initialValue), y(initialValue), z(initialValue)
		{
			//DCHECK(hasNans());
		}
		Vector3(T initialX, T initialY, T initialZ)
			: x(initialX), y(initialY), z(initialZ)
		{
			//DCHECK(hasNans());
		}

		Float lengthSquared() const
		{
			return (x * x + y * y + z * z);
		}

		Float length() const
		{
			return (std::sqrt(lengthSquared()));
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

		Vector3<T> operator+(const Vector3<T> &v) const
		{
			return (Vector3<T>(x + v.x, y + v.y, z + v.z));
		}

		Vector3<T> &operator+=(const Vector3<T> &v)
		{
			x += v.x;
			y += v.y;
			z += v.z;
			return (*this);
		}

		Vector3<T> operator-() const
		{
			return (Vector3<T>(-x, -y, -z));
		}

		Vector3<T> operator-(const Vector3<T> &v) const
		{
			return (Vector3<T>(x - v.x, y - v.y, z - v.z));
		}

		Vector3<T> &operator-=(const Vector3<T> &v)
		{
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return (*this);
		}

		Vector3<T> operator*(T scalar) const
		{
			return (Vector3<T>(x * scalar, y * scalar, z * scalar));
		}

		Vector3<T> &operator*=(T scalar)
		{
			x *= scalar;
			y *= scalar;
			z *= scalar;
			return (*this);
		}

		Vector3<T> operator/(T scalar) const
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			return (Vector3<T>(x * inv, y * inv, z * inv));
		}

		Vector3<T> &operator/=(T scalar)
		{
			DCHECK(scalar != 0);
			Float inv = static_cast<Float>(1) / scalar;
			x *= inv;
			y *= inv;
			z *= inv;
			return (*this);
		}

		bool operator==(const Vector3<T> &v) const
		{
			return (x == v.x && y == v.y && z == v.z);
		}

		bool operator!=(const Vector3<T> &v) const
		{
			return (!(*this == v));
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
	};

	template <typename T>
	inline T dot(const Vector2<T> &v1, const Vector2<T> &v2)
	{
		return (sumOfProducts(v1.x, v2.x, v1.y, v2.y));
	}

	template <typename T>
	inline T dot(const Vector3<T> &v1, const Vector3<T> &v2)
	{
 		return (std::fma(v1.x, v2.x, sumOfProducts(v1.y, v2.y, v1.z, v2.z)));
	}

	template <typename T>
	inline Vector3<T> cross(const Vector3<T> &v1, const Vector3<T> &v2)
	{
		return (Vector3<T>(differenceOfProducts(v1.y, v2.z, v1.z, v2.y),
			differenceOfProducts(v1.z, v2.x, v1.x, v2.z),
			differenceOfProducts(v1.x, v2.y, v1.y, v2.x)));
	}

	template <typename T>
	Vector2<T> normalize(const Vector2<T> &v)
	{
		return (v / v.length());
	}

	template <typename T>
	Vector3<T> normalize(const Vector3<T> &v)
	{
		return (v / v.length());
	}

	template <typename T>
	Vector2<T> min(const Vector2<T> &v1, const Vector2<T> &v2)
	{
		return (Vector2<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y)));
	}

	template <typename T>
	Vector3<T> min(const Vector3<T> &v1, const Vector3<T> &v2)
	{
		return (Vector3<T>(std::min(v1.x, v2.x), std::min(v1.y, v2.y), std::min(v1.z, v2.z)));
	}

	template <typename T>
	Vector2<T> max(const Vector2<T> &v1, const Vector2<T> &v2)
	{
		return (Vector2<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y)));
	}

	template <typename T>
	Vector3<T> max(const Vector3<T> &v1, const Vector3<T> &v2)
	{
		return (Vector3<T>(std::max(v1.x, v2.x), std::max(v1.y, v2.y), std::max(v1.z, v2.z)));
	}

	template <typename T>
	Vector2<T> abs(const Vector2<T> &v)
	{
		return (Vector2<T>(std::abs(v.x), std::abs(v.y)));
	}

	template <typename T>
	Vector3<T> abs(const Vector3<T> &v)
	{
		return (Vector3<T>(std::abs(v.x), std::abs(v.y), std::abs(v.z)));
	}

	template <typename T>
	inline Vector2<T> operator*(T scalar, const Vector2<T> &v)
	{
		return (v * scalar);
	}

	template <typename T>
	inline Vector3<T> operator*(T scalar, const Vector3<T> &v)
	{
		return (v * scalar);
	}

	template <typename T>
	Vector2<T> permute(const Vector2<T> &v, int x, int y) {
		return Vector2<T>(v[x], v[y]);
	}

	template <typename T>
	Vector3<T> permute(const Vector3<T> &v, int x, int y, int z) {
		return Vector3<T>(v[x], v[y], v[z]);
	}

	template <typename T>
	std::ostream &operator<<(std::ostream &os, const Vector2<T> &v)
	{
		os << "{" << v.x << ", " << v.y << "}";
		return (os);
	}

	template <typename T>
	std::ostream &operator<<(std::ostream &os, const Vector3<T> &v)
	{
		os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
		return (os);
	}
		
	typedef Vector2<Float> Vec2;
	typedef Vector2<Float> Vec2f;
	typedef Vector2<int32_t> Vec2i;
	typedef Vector3<Float> Vec3;
	typedef Vector3<Float> Vec3f;
	typedef Vector3<int32_t> Vec3i;

	typedef Vector3<Float> Normal;
}