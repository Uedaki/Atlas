#pragma once

#include "atlas/Atlas.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
    class Filter
    {
    public:
        virtual ~Filter() = default;
        Filter(const Vec2f &radius)
            : radius(radius), invRadius(Vec2f(1 / radius.x, 1 / radius.y)) {}
        virtual Float evaluate(const Point2f &p) const = 0;

        const Vec2f radius;
        const Vec2f invRadius;
    };

    class BoxFilter : public Filter {
    public:
        BoxFilter(const Vec2f &radius)
            : Filter(radius)
        {}

        Float evaluate(const Point2f &p) const override
        {
            return (1.f);
        }
    };
}