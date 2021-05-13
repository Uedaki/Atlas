#pragma once

#include "atlas/Atlas.h"

namespace atlas
{
    class EFloat
    {
    public:
        EFloat() {}
        EFloat(float v, float err = 0.f) : v(v)
        {
            if (err == 0.)
                low = high = v;
            else {
                low = nextFloatDown(v - err);
                high = nextFloatUp(v + err);
            }
#ifndef NDEBUG
            vPrecise = v;
            Check();
#endif  // NDEBUG
        }

#ifndef NDEBUG
        EFloat(float v, long double lD, float err) : EFloat(v, err) {
            vPrecise = lD;
            Check();
        }
#endif  // DEBUG

        EFloat operator+(EFloat ef) const {
            EFloat r;
            r.v = v + ef.v;
#ifndef NDEBUG
            r.vPrecise = vPrecise + ef.vPrecise;
#endif  // DEBUG

            r.low = nextFloatDown(lowerBound() + ef.lowerBound());
            r.high = nextFloatUp(upperBound() + ef.upperBound());
            r.Check();
            return r;
        }

        explicit operator float() const
        {
            return v;
        }
        
        explicit operator double() const
        {
            return v;
        }
        
        float GetAbsoluteError() const
        {
            return nextFloatUp(std::max(std::abs(high - v),
                std::abs(v - low)));
        }

        float upperBound() const
        {
            return high;
        }

        float lowerBound() const
        {
            return low;
        }

#ifndef NDEBUG
        float GetRelativeError() const
        {
            return ((float)(std::abs((vPrecise - v) / vPrecise)));
        }
        
        long double PreciseValue() const
        {
            return vPrecise;
        }
#endif
        EFloat operator-(EFloat ef) const
        {
            EFloat r;
            r.v = v - ef.v;
#ifndef NDEBUG
            r.vPrecise = vPrecise - ef.vPrecise;
#endif
            r.low = nextFloatDown(lowerBound() - ef.upperBound());
            r.high = nextFloatUp(upperBound() - ef.lowerBound());
            r.Check();
            return r;
        }

        EFloat operator*(EFloat ef) const
        {
            EFloat r;
            r.v = v * ef.v;
#ifndef NDEBUG
            r.vPrecise = vPrecise * ef.vPrecise;
#endif
            Float prod[4] = {
                lowerBound() * ef.lowerBound(), upperBound() * ef.lowerBound(),
                lowerBound() * ef.upperBound(), upperBound() * ef.upperBound() };
            r.low = nextFloatDown(
                std::min(std::min(prod[0], prod[1]), std::min(prod[2], prod[3])));
            r.high = nextFloatUp(
                std::max(std::max(prod[0], prod[1]), std::max(prod[2], prod[3])));
            r.Check();
            return r;
        }

        EFloat operator/(EFloat ef) const
        {
            EFloat r;
            r.v = v / ef.v;
#ifndef NDEBUG
            r.vPrecise = vPrecise / ef.vPrecise;
#endif
            if (ef.low < 0 && ef.high > 0)
            {
                r.low = -INFINITY;
                r.high = INFINITY;
            }
            else
            {
                Float div[4] = {
                    lowerBound() / ef.lowerBound(), upperBound() / ef.lowerBound(),
                    lowerBound() / ef.upperBound(), upperBound() / ef.upperBound() };
                r.low = nextFloatDown(
                    std::min(std::min(div[0], div[1]), std::min(div[2], div[3])));
                r.high = nextFloatUp(
                    std::max(std::max(div[0], div[1]), std::max(div[2], div[3])));
            }
            r.Check();
            return r;
        }

        EFloat operator-() const
        {
            EFloat r;
            r.v = -v;
#ifndef NDEBUG
            r.vPrecise = -vPrecise;
#endif
            r.low = -high;
            r.high = -low;
            r.Check();
            return r;
        }

        inline bool operator==(EFloat fe) const
        {
            return v == fe.v;
        }
        inline void Check() const {
//            if (!std::isinf(low) && !std::isnan(low) && !std::isinf(high) &&
//                !std::isnan(high))
//                CHECK(low, high);
//#ifndef NDEBUG
//            if (!std::isinf(v) && !std::isnan(v)) {
//                CHECK(lowerBound(), vPrecise);
//                CHECK(vPrecise, upperBound());
//            }
//#endif
        }

        EFloat(const EFloat &ef)
        {
            ef.Check();
            v = ef.v;
            low = ef.low;
            high = ef.high;
#ifndef NDEBUG
            vPrecise = ef.vPrecise;
#endif
        }

        EFloat &operator=(const EFloat &ef)
        {
            ef.Check();
            if (&ef != this) {
                v = ef.v;
                low = ef.low;
                high = ef.high;
#ifndef NDEBUG
                vPrecise = ef.vPrecise;
#endif
            }
            return *this;
        }

    private:
        float v;
        float low;
        float high;
#ifndef NDEBUG
        long double vPrecise;
#endif  // NDEBUG
        friend inline EFloat sqrt(EFloat fe);
        friend inline EFloat abs(EFloat fe);
        friend inline bool quadratic(EFloat a, EFloat b, EFloat c, EFloat *t0, EFloat *t1);
    };

    // EFloat Inline Functions
    inline EFloat operator*(float f, EFloat fe)
    {
        return EFloat(f) * fe;
    }

    inline EFloat operator/(float f, EFloat fe)
    {
        return EFloat(f) / fe;
    }

    inline EFloat operator+(float f, EFloat fe)
    {
        return EFloat(f) + fe;
    }

    inline EFloat operator-(float f, EFloat fe)
    {
        return EFloat(f) - fe;
    }

    inline EFloat sqrt(EFloat fe)
    {
        EFloat r;
        r.v = std::sqrt(fe.v);
#ifndef NDEBUG
        r.vPrecise = std::sqrt(fe.vPrecise);
#endif
        r.low = nextFloatDown(std::sqrt(fe.low));
        r.high = nextFloatUp(std::sqrt(fe.high));
        r.Check();
        return r;
    }

    inline EFloat abs(EFloat fe)
    {
        if (fe.low >= 0)
            return fe;
        else if (fe.high <= 0)
        {
            EFloat r;
            r.v = -fe.v;
#ifndef NDEBUG
            r.vPrecise = -fe.vPrecise;
#endif
            r.low = -fe.high;
            r.high = -fe.low;
            r.Check();
            return r;
        }
        else
        {
            EFloat r;
            r.v = std::abs(fe.v);
#ifndef NDEBUG
            r.vPrecise = std::abs(fe.vPrecise);
#endif
            r.low = 0;
            r.high = std::max(-fe.low, fe.high);
            r.Check();
            return r;
        }
    }

    inline bool quadratic(EFloat a, EFloat b, EFloat c, EFloat *t0, EFloat *t1);
    inline bool quadratic(EFloat a, EFloat b, EFloat c, EFloat *t0, EFloat *t1)
    {
        double discrim = (double)b.v * (double)b.v - 4. * (double)a.v * (double)c.v;
        if (discrim < 0.)
            return false;
        double rootDiscrim = std::sqrt(discrim);

        EFloat floatRootDiscrim((float)rootDiscrim, (float)(MACHINE_EPSILON * rootDiscrim));

        EFloat q;
        if ((float)b < 0)
            q = -.5 * (b - floatRootDiscrim);
        else
            q = -.5 * (b + floatRootDiscrim);
        *t0 = q / a;
        *t1 = c / q;
        if ((float)*t0 > (float)*t1)
            std::swap(*t0, *t1);
        return true;
    }
}