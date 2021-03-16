#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
    inline Vec3f sphericalDirection(Float sinTheta, Float cosTheta, Float phi)
    {
        return Vec3f(sinTheta * std::cos(phi), sinTheta * std::sin(phi),
            cosTheta);
    }

    inline Vec3f sphericalDirection(Float sinTheta, Float cosTheta, Float phi, const Vec3f &x, const Vec3f &y, const Vec3f &z)
    {
        return sinTheta * std::cos(phi) * x + sinTheta * std::sin(phi) * y +
            cosTheta * z;
    }

    template <typename T>
    inline void coordinateSystem(const Vector3<T> &v1, Vector3<T> *v2, Vector3<T> *v3)
    {
        if (std::abs(v1.x) > std::abs(v1.y))
            *v2 = Vector3<T>(-v1.z, 0, v1.x) / std::sqrt(v1.x * v1.x + v1.z * v1.z);
        else
            *v2 = Vector3<T>(0, v1.z, -v1.y) / std::sqrt(v1.y * v1.y + v1.z * v1.z);
        *v3 = cross(v1, *v2);
    }

    inline Point3f offsetRayOrigin(const Point3f &p, const Vec3f &pError, const Normal &n, const Vec3f &w)
    {
        Float d = dot(abs(n), pError);
        Vec3f offset = d * Vec3f(n);
        if (dot(w, n) < 0)
            offset = -offset;
        Point3f po = p + offset;
        for (int i = 0; i < 3; ++i)
        {
            if (offset[i] > 0)
                po[i] = nextFloatUp(po[i]);
            else if (offset[i] < 0)
                po[i] = nextFloatDown(po[i]);
        }
        return po;
    }

    inline Normal faceForward(const Normal &n, const Normal &n2)
    {
        return (dot(n, n2) < 0.f) ? -n : n;
    }
}