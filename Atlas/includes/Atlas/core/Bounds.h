#pragma once

#include <algorithm>
#include <cstdint>
#include <limits>

#include "atlas/Atlas.h"
#include "atlas/core/Logging.h"
#include "atlas/core/Math.h"
#include "atlas/core/Points.h"
#include "atlas/core/Ray.h"

namespace atlas
{
	template <typename T>
	struct Bounds2
	{
		Point2<T> min = Point2<T>(std::numeric_limits<T>::lowest());
		Point2<T> max = Point2<T>(std::numeric_limits<T>::max());

		Bounds2() = default;
		Bounds2(const Point2<T> &p)
			: min(p), max(p)
		{}
		Bounds2(const Point2<T> &p1, const Point2<T> &p2)
			: min(std::min(p1.x, p2.x), std::min(p1.y, p2.y))
			, max(std::max(p1.x, p2.x), std::max(p1.y, p2.y))
		{}

		template <typename U>
		Bounds2(const Bounds2<U> &b)
			: min(b.min), max(b.max)
		{}

		Vector2<T> diagonal() const
		{
			return (max - min);
		}

		T surfaceArea() const
		{
			Vector2<T> d = diagonal();
			return (d.x * d.y);
		}

		int8_t maxExtent() const
		{
			Vector2<T> d = diagonal();
			return (d.x > d.y ? 0 : 1);
		}

		Point2<T> interpolate(const Point2f &t) const
		{
			return (Point2<T>(lerp(min.x, max.x, t.x),
				lerp(min.y, max.y, t.y)));
		}

		Vector2<T> offset(const Point2<T> &p) const
		{
			Vector2<T> o = p - min;
			if (max.x > min.x)
				o.x /= max.x - min.x;
			if (max.y > min.y)
				o.y /= max.y - min.y;
			return (o);
		}

		const Point2<T> &operator[](int8_t i) const
		{
			DCHECK(0 <= i && i <= 2);
			if (i == 0)
				return (min);
			return (max);
		}

		Point2<T> &operator[](int8_t i)
		{
			DCHECK(0 <= i && i <= 2);
			if (i == 0)
				return (min);
			return (max);
		}
	};

	template <typename T>
	struct Bounds3
	{
		Point3<T> min = Point3<T>(std::numeric_limits<T>::max());
		Point3<T> max = Point3<T>(std::numeric_limits<T>::lowest());

		Bounds3() = default;
		Bounds3(const Point3<T> & p)
			: min(p), max(p)
		{}
		Bounds3(const Point3<T> & p1, const Point3<T> & p2)
			: min(std::min(p1.x, p2.x), std::min(p1.y, p2.y), std::min(p1.z, p2.z))
			, max(std::max(p1.x, p2.x), std::max(p1.y, p2.y), std::max(p1.z, p2.z))
		{}

		template <typename U>
		Bounds3(const Bounds3<U> &b)
			: min(b.min), max(b.max)
		{}

		Vector3<T> diagonal() const
		{
			return (max - min);
		}

		T surfaceArea() const
		{
			Vector3<T> d = diagonal();
			return (2 * (d.x * d.y + d.x * d.z + d.y * d.z));
		}

		T volume() const
		{
			Vector3<T> d = diagonal();
			return (d.x * d.y * d.z);
		}

		int8_t maxExtent() const
		{
			Vector3<T> d = diagonal();
			return (d.x > d.y ? (d.x > d.z ? 0 : 2) : (d.y > d.z ? 1 : 2));
		}

		Point3<T> interpolate(const Point3f &t) const
		{
			return (Point3<T>(lerp(min.x, max.x, t.x),
				lerp(min.y, max.y, t.y),
				lerp(min.z, max.z, t.z)));
		}

		Vector3<T> offset(const Point3<T> &p) const
		{
			Vector3<T> o = p - min;
			if (max.x > min.x)
				o.x /= max.x - min.x;
			if (max.y > min.y)
				o.y /= max.y - min.y;
			if (max.z > min.z)
				o.z /= max.z - min.z;
			return (o);
		}

		const Point3<T> &operator[](int8_t i) const
		{
			DCHECK(0 <= i && i <= 2);
			if (i == 0)
				return (min);
			return (max);
		}

		Point3<T> &operator[](int8_t i)
		{
			DCHECK(0 <= i && i <= 2);
			if (i == 0)
				return (min);
			return (max);
		}

		inline bool intersectP(const Ray &ray, const Vec3f &invDir, const int8_t dirIsNeg[3]) const
		{
			//const Bounds3<Float> &bounds = *this;
			//
			//Float tMin = (bounds[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
			//Float tMax = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x;
			//Float tyMin = (bounds[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
			//Float tyMax = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y;

			//tMax *= 1 + 2 * gamma(3);
			//tyMax *= 1 + 2 * gamma(3);
			//if (tMin > tyMax || tyMin > tMax)
			//	return false;
			//if (tyMin > tMin)
			//	tMin = tyMin;
			//if (tyMax < tMax)
			//	tMax = tyMax;

			//Float tzMin = (bounds[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
			//Float tzMax = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z;

			//tzMax *= 1 + 2 * gamma(3);
			//if (tMin > tzMax || tzMin > tMax)
			//	return false;
			//if (tzMin > tMin)
			//	tMin = tzMin;
			//if (tzMax < tMax)
			//	tMax = tzMax;
			//return (tMin < ray.tmax) && (tMax > 0);


			float tmin;
			float tmax;
			float tymin;
			float tymax;
			float tzmin;
			float tzmax;

			const Bounds3<Float> &bounds = *this;

			tmin = (bounds[dirIsNeg[0]].x - ray.origin.x) * invDir.x;
			tmax = (bounds[1 - dirIsNeg[0]].x - ray.origin.x) * invDir.x;
			tymin = (bounds[dirIsNeg[1]].y - ray.origin.y) * invDir.y;
			tymax = (bounds[1 - dirIsNeg[1]].y - ray.origin.y) * invDir.y;

			if (tmin > tymax || tymin > tmax)
			{
				return (false);
			}
			if (tymin > tmin)
				tmin = tymin;
			if (tymax < tmax)
				tmax = tymax;

			tzmin = (bounds[dirIsNeg[2]].z - ray.origin.z) * invDir.z;
			tzmax = (bounds[1 - dirIsNeg[2]].z - ray.origin.z) * invDir.z;

			if (tmin > tzmax || tzmin > tmax)
			{
				return (false);
			}
			if (tzmin > tmin)
				tmin = tzmin;
			if (tzmax < tmax)
				tmax = tzmax;

			if (tmin < ray.tmax && 0 < tmax)
				return (true);
			return (false);
		}
	};

	template <typename T, typename U>
	inline Bounds2<T> expand(const Bounds2<T> &b, U delta)
	{
		return (Bounds2<T>(b.min - Vector2<T>(delta),
			b.max + Vector2<T>(delta)));
	}

	template <typename T>
	Bounds2<T> expand(const Bounds2<T> &b, const Point2<T> &p)
	{
		return (Bounds2<T>(Point2<T>(std::min(b.min.x, p.x), std::min(b.min.y, p.y)),
			Point2<T>(std::max(b.max.x, p.x), std::max(b.max.y, p.y))));
	}

	template <typename T>
	Bounds2<T> expand(const Bounds2<T> &b1, const Bounds2<T> &b2)
	{
		return (Bounds2<T>(Point2<T>(std::min(b1.min.x, b2.min.x), std::min(b1.min.y, b2.min.y)),
			Point2<T>(std::max(b1.max.x, b2.max.x), std::max(b1.max.z, b2.max.z))));
	}

	template <typename T, typename U>
	inline Bounds3<T> expand(const Bounds3<T> &b, U delta)
	{
		return (Bounds3<T>(b.min - Vector3<T>(delta),
			b.max + Vector3<T>(delta)));
	}

	template <typename T>
	Bounds3<T> expand(const Bounds3<T> &b, const Point3<T> &p)
	{
		return (Bounds3<T>(Point3<T>(std::min(b.min.x, p.x), std::min(b.min.y, p.y), std::min(b.min.z, p.z)),
			Point3<T>(std::max(b.max.x, p.x), std::max(b.max.y, p.y), std::max(b.max.z, p.z))));
	}

	template <typename T>
	Bounds3<T> expand(const Bounds3<T> &b1, const Bounds3<T> &b2)
	{
		return (Bounds3<T>(Point3<T>(std::min(b1.min.x, b2.min.x), std::min(b1.min.y, b2.min.y), std::min(b1.min.z, b2.min.z)),
			Point3<T>(std::max(b1.max.x, b2.max.x), std::max(b1.max.y, b2.max.y), std::max(b1.max.z, b2.max.z))));
	}

	template <typename T>
	Bounds2<T> intersect(const Bounds2<T> &b1, const Bounds2<T> &b2)
	{
		return (Bounds2<T>(Point2<T>(std::max(b1.min.x, b2.min.x), std::max(b1.min.y, b2.min.y)),
			Point2<T>(std::min(b1.max.x, b2.max.x), std::min(b1.max.z, b2.max.z))));
	}

	template <typename T>
	Bounds3<T> intersect(const Bounds3<T> &b1, const Bounds3<T> &b2)
	{
		return (Bounds3<T>(Point3<T>(std::max(b1.min.x, b2.min.x), std::max(b1.min.y, b2.min.y), std::max(b1.min.z, b2.min.z)),
			Point3<T>(std::min(b1.max.x, b2.max.x), std::min(b1.max.z, b2.max.z), std::min(b1.max.z, b2.max.z))));
	}

	template <typename T>
	bool overlaps(const Bounds2<T> &b1, const Bounds2<T> &b2)
	{
		bool x = (b1.max.x >= b2.min.x) && (b1.min.x <= b2.max.x);
		bool y = (b1.max.y >= b2.min.y) && (b1.min.y <= b2.max.y);
		return (x && y);
	}

	template <typename T>
	bool overlaps(const Bounds3<T> &b1, const Bounds3<T> &b2)
	{
		bool x = (b1.max.x >= b2.min.x) && (b1.min.x <= b2.max.x);
		bool y = (b1.max.y >= b2.min.y) && (b1.min.y <= b2.max.y);
		bool z = (b1.max.z >= b2.min.z) && (b1.min.z <= b2.max.z);
		return (x && y && z);
	}

	template <typename T>
	bool inside(const Bounds2<T> &b, const Point2<T> &p)
	{
		bool x = (b.max.x >= p.x) && (b.min.x <= p.x);
		bool y = (b.max.y >= p.y) && (b.min.y <= p.y);
		return (x && y);
	}

	template <typename T>
	bool insideExclusive(const Bounds2<T> &b, const Point2<T> &p)
	{
		bool x = (b.max.x > p.x) && (b.min.x <= p.x);
		bool y = (b.max.y > p.y) && (b.min.y <= p.y);
		return (x && y);
	}

	template <typename T>
	bool inside(const Bounds3<T> &b, const Point3<T> &p)
	{
		bool x = (b.max.x >= p.x) && (b.min.x <= p.x);
		bool y = (b.max.y >= p.y) && (b.min.y <= p.y);
		bool z = (b.max.z >= p.z) && (b.min.z <= p.z);
		return (x && y && z);
	}

	template <typename T>
	bool insideExclusive(const Bounds3<T> &b, const Point3<T> &p)
	{
		bool x = (b.max.x > p.x) && (b.min.x <= p.x);
		bool y = (b.max.y > p.y) && (b.min.y <= p.y);
		bool z = (b.max.z > p.z) && (b.min.z <= p.z);
		return (x && y && z);
	}

	typedef Bounds2<Float> Bounds2f;
	typedef Bounds2<int32_t> Bounds2i;
	typedef Bounds3<Float> Bounds3f;
	typedef Bounds3<int32_t> Bounds3i;

	class Bounds2iIterator : public std::forward_iterator_tag
	{
	public:
		Bounds2iIterator(const Bounds2i &b, const Point2i &pt)
			: p(pt), bounds(&b)
		{}

		Bounds2iIterator operator++()
		{
			advance();
			return *this;
		}

		Bounds2iIterator operator++(int)
		{
			Bounds2iIterator old = *this;
			advance();
			return old;
		}
		
		bool operator==(const Bounds2iIterator &bi) const
		{
			return p == bi.p && bounds == bi.bounds;
		}
		
		bool operator!=(const Bounds2iIterator &bi) const
		{
			return p != bi.p || bounds != bi.bounds;
		}

		Point2i operator*() const
		{
			return p;
		}

	private:
		void advance()
		{
			++p.x;
			if (p.x == bounds->max.x)
			{
				p.x = bounds->min.x;
				++p.y;
			}
		}

		Point2i p;
		const Bounds2i *bounds;
	};

	inline Bounds2iIterator begin(const Bounds2i &b)
	{
		return Bounds2iIterator(b, b.min);
	}

	inline Bounds2iIterator end(const Bounds2i &b)
	{
		Point2i pEnd(b.min.x, b.max.y);
		if (b.min.x >= b.max.x || b.min.y >= b.max.y)
			pEnd = b.min;
		return Bounds2iIterator(b, pEnd);
	}
}