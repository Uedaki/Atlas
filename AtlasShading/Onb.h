#pragma once

#include "atlas/core/Vectors.h"

namespace atlas
{
	struct Onb
	{
	public:
		Onb() = default;
		Onb(const Vec3f &n)
		{
			buildFromW(n);
		}

		Vec3f local(Float a, Float b, Float c) const
		{
			return (a * u + b * v + c * w);
		}

		Vec3f local(const Vec3f &a) const
		{
			return (a.x * u + a.y * v + a.z * w);
		}

		void buildFromW(const Vec3f &n)
		{
			w = normalize(n);
			Vec3f a = (std::abs(w.x) > 0.9) ? Vec3f(0, 1, 0) : Vec3f(1, 0, 0);
			v = normalize(cross(w, a));
			u = cross(w, v);
		}

		inline Vec3f &operator[](int i)
		{
			if (i == 0)
				return (u);
			else if (i == 1)
				return (v);
			else
				return (w);
		}

		inline const Vec3f &operator[](int i) const
		{
			if (i == 0)
				return (u);
			else if (i == 1)
				return (v);
			else
				return (w);
		}

		Vec3f u;
		Vec3f v;
		Vec3f w;
	};
}