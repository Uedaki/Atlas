#include "atlas/core/Microfacet.h"

#include "atlas/core/BxDF.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Math.h"
#include "atlas/core/Reflection.h"

using namespace atlas;

Float MicrofacetDistribution::pdf(const Vec3f &wo, const Vec3f &wh) const
{
    if (sampleVisibleArea)
        return d(wh) * g1(wo) * std::abs(dot(wo, wh)) / absCosTheta(wo);
    else
        return d(wh) * absCosTheta(wh);
}

namespace
{
    inline Float ErfInv(Float x)
    {
        Float w, p;
        x = atlas::clamp(x, -.99999f, .99999f);
        w = -std::log((1 - x) * (1 + x));
        if (w < 5) {
            w = w - 2.5f;
            p = 2.81022636e-08f;
            p = 3.43273939e-07f + p * w;
            p = -3.5233877e-06f + p * w;
            p = -4.39150654e-06f + p * w;
            p = 0.00021858087f + p * w;
            p = -0.00125372503f + p * w;
            p = -0.00417768164f + p * w;
            p = 0.246640727f + p * w;
            p = 1.50140941f + p * w;
        }
        else {
            w = std::sqrt(w) - 3;
            p = -0.000200214257f;
            p = 0.000100950558f + p * w;
            p = 0.00134934322f + p * w;
            p = -0.00367342844f + p * w;
            p = 0.00573950773f + p * w;
            p = -0.0076224613f + p * w;
            p = 0.00943887047f + p * w;
            p = 1.00167406f + p * w;
            p = 2.83297682f + p * w;
        }
        return p * x;
    }

    inline Float Erf(Float x)
    {
        // constants
        Float a1 = 0.254829592f;
        Float a2 = -0.284496736f;
        Float a3 = 1.421413741f;
        Float a4 = -1.453152027f;
        Float a5 = 1.061405429f;
        Float p = 0.3275911f;

        // Save the sign of x
        int sign = 1;
        if (x < 0) sign = -1;
        x = std::abs(x);

        // A&S formula 7.1.26
        Float t = 1 / (1 + p * x);
        Float y =
            1 -
            (((((a5 * t + a4) * t) + a3) * t + a2) * t + a1) * t * std::exp(-x * x);

        return sign * y;
    }

    void beckmannSample11(Float cosThetaI, Float u1, Float u2, Float &slopeX, Float &slopeY) 
    {
        if (cosThetaI > .9999)
        {
            Float r = std::sqrt(-std::log(1.0f - u1));
            Float sinPhi = std::sin(2 * PI * u2);
            Float cosPhi = std::cos(2 * PI * u2);
            slopeX = r * cosPhi;
            slopeY = r * sinPhi;
            return;
        }

        Float sinThetaI = std::sqrt(std::max((Float)0, (Float)1 - cosThetaI * cosThetaI));
        Float tanThetaI = sinThetaI / cosThetaI;
        Float cotThetaI = 1 / tanThetaI;

        Float a = -1, c = Erf(cotThetaI);
        Float sample_x = std::max(u1, (Float)1e-6f);

        Float thetaI = std::acos(cosThetaI);
        Float fit = 1 + thetaI * (-0.876f + thetaI * (0.4265f - 0.0594f * thetaI));
        Float b = c - (1 + c) * std::pow(1 - sample_x, fit);

        static const Float SQRT_PI_INV = 1.f / std::sqrt(PI);
        Float normalization = 1. / (1. + c + SQRT_PI_INV * tanThetaI * std::exp(-cotThetaI * cotThetaI));

        int it = 0;
        while (++it < 10)
        {
            if (!(b >= a && b <= c))
                b = 0.5f * (a + c);

            Float invErf = ErfInv(b);
            Float value = normalization * (1 + b + SQRT_PI_INV * tanThetaI * std::exp(-invErf * invErf)) - sample_x;
            Float derivative = normalization * (1 - invErf * tanThetaI);

            if (std::abs(value) < 1e-5f)
                break;

            if (value > 0)
                c = b;
            else
                a = b;

            b -= value / derivative;
        }

        slopeX = ErfInv(b);

        slopeY = ErfInv(2.0f * std::max(u2, (Float)1e-6f) - 1.0f);

        CHECK(!std::isinf(slopeX));
        CHECK(!std::isnan(slopeX));
        CHECK(!std::isinf(slopeY));
        CHECK(!std::isnan(slopeY));
    }

    Vec3f beckmannSample(const Vec3f &wi, Float alphaX, Float alphaY, Float u1, Float u2)
    {
        Vec3f wiStretched = normalize(Vec3f(alphaX * wi.x, alphaY * wi.y, wi.z));

        Float slope_x, slope_y;
        beckmannSample11(cosTheta(wiStretched), u1, u2, slope_x, slope_y);

        Float tmp = cosPhi(wiStretched) * slope_x - sinPhi(wiStretched) * slope_y;
        slope_y = sinPhi(wiStretched) * slope_x + cosPhi(wiStretched) * slope_y;
        slope_x = tmp;

        slope_x = alphaX * slope_x;
        slope_y = alphaY * slope_y;

        return (normalize(Vec3f(-slope_x, -slope_y, 1.f)));
    }
}

Float BeckmannDistribution::d(const Vec3f &wh) const
{
    Float tan2T = tan2Theta(wh);
    if (std::isinf(tan2T))
        return 0.;
    Float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
    return std::exp(-tan2T * (cos2Phi(wh) / (alphaX * alphaX) +
        sin2Phi(wh) / (alphaY * alphaY))) /
        (PI * alphaX * alphaY * cos4Theta);
}

Float BeckmannDistribution::lambda(const Vec3f &w) const {
    Float absTanTheta = std::abs(tanTheta(w));
    if (std::isinf(absTanTheta))
        return 0.;
   
    Float alpha =
        std::sqrt(cos2Phi(w) * alphaX * alphaX + sin2Phi(w) * alphaY * alphaY);
    Float a = 1 / (alpha * absTanTheta);
    if (a >= 1.6f)
        return 0;
    return (1 - 1.259f * a + 0.396f * a * a) / (3.535f * a + 2.181f * a * a);
}

Vec3f BeckmannDistribution::sampleWh(const Vec3f &wo, const Point2f &u) const
{
    if (!sampleVisibleArea)
    {
        Float tan2Theta, phi;
        if (alphaX == alphaY)
        {
            Float logSample = std::log(1 - u[0]);
            DCHECK(!std::isinf(logSample));
            tan2Theta = -alphaX * alphaX * logSample;
            phi = u[1] * 2 * PI;
        }
        else
        {
            Float logSample = std::log(1 - u[0]);
            DCHECK(!std::isinf(logSample));
            phi = std::atan(alphaY / alphaX *
                std::tan(2 * PI * u[1] + 0.5f * PI));
            if (u[1] > 0.5f)
                phi += PI;
            Float sinPhi = std::sin(phi);
            Float cosPhi = std::cos(phi);
            Float alphax2 = alphaX * alphaX, alphay2 = alphaY * alphaY;
            tan2Theta = -logSample /
                (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
        }

        Float cosTheta = 1. / std::sqrt(1. + tan2Theta);
        Float sinTheta = std::sqrt(std::max((Float)0, 1 - cosTheta * cosTheta));
        Vec3f wh = sphericalDirection(sinTheta, cosTheta, phi);
        if (!sameHemisphere(wo, wh))
            wh = -wh;
        return (wh);
    }
    else
    {
        Vec3f wh;
        bool flip = wo.z < 0;
        wh = beckmannSample(flip ? -wo : wo, alphaX, alphaY, u[0], u[1]);
        if (flip)
            wh = -wh;
        return (wh);
    }
}

namespace
{
    void trowbridgeReitzSample11(Float cosTheta, Float u1, Float u2, Float &slopeX, Float &slopeY)
    {
        if (cosTheta > .9999)
        {
            Float r = sqrt(u1 / (1 - u1));
            Float phi = 6.28318530718 * u2;
            slopeX = r * cos(phi);
            slopeY = r * sin(phi);
            return;
        }

        Float sinTheta = std::sqrt(std::max((Float)0, (Float)1 - cosTheta * cosTheta));
        Float tanTheta = sinTheta / cosTheta;
        Float a = 1 / tanTheta;
        Float G1 = 2 / (1 + std::sqrt(1.f + 1.f / (a * a)));

        Float A = 2 * u1 / G1 - 1;
        Float tmp = 1.f / (A * A - 1.f);
        if (tmp > 1e10)
            tmp = 1e10;
        Float B = tanTheta;
        Float D = std::sqrt(std::max(Float(B * B * tmp * tmp - (A * A - B * B) * tmp), Float(0)));
        Float slope_x_1 = B * tmp - D;
        Float slope_x_2 = B * tmp + D;
        slopeX = (A < 0 || slope_x_2 > 1.f / tanTheta) ? slope_x_1 : slope_x_2;

        Float S;
        if (u2 > 0.5f)
        {
            S = 1.f;
            u2 = 2.f * (u2 - .5f);
        }
        else
        {
            S = -1.f;
            u2 = 2.f * (.5f - u2);
        }
        Float z = (u2 * (u2 * (u2 * 0.27385f - 0.73369f) + 0.46341f)) /
            (u2 * (u2 * (u2 * 0.093073f + 0.309420f) - 1.000000f) + 0.597999f);
        slopeY = S * z * std::sqrt(1.f + slopeX * slopeX);

        CHECK(!std::isinf(slopeY));
        CHECK(!std::isnan(slopeY));
    }

    Vec3f trowbridgeReitzSample(const Vec3f &wi, Float alphaX, Float alphaY, Float U1, Float U2)
    {
        Vec3f wiStretched = normalize(Vec3f(alphaX * wi.x, alphaY * wi.y, wi.z));

        Float slopeX, slopeY;
        trowbridgeReitzSample11(cosTheta(wiStretched), U1, U2, slopeX, slopeY);

        Float tmp = cosPhi(wiStretched) * slopeX - sinPhi(wiStretched) * slopeY;
        slopeY = sinPhi(wiStretched) * slopeX + cosPhi(wiStretched) * slopeY;
        slopeX = tmp;

        slopeX = alphaX * slopeX;
        slopeY = alphaY * slopeY;

        return (normalize(Vec3f(-slopeX, -slopeY, 1.)));
    }
}

Float TrowbridgeReitzDistribution::d(const Vec3f &wh) const
{
    Float tan2T = tan2Theta(wh);
    if (std::isinf(tan2T))
        return (0.);
    const Float cos4Theta = cos2Theta(wh) * cos2Theta(wh);
    Float e = (cos2Phi(wh) / (alphaX * alphaX) + sin2Phi(wh) / (alphaY * alphaY)) * tan2T;
    return (1 / (PI * alphaX * alphaY * cos4Theta * (1 + e) * (1 + e)));
}

Float TrowbridgeReitzDistribution::lambda(const Vec3f &w) const
{
    Float absTanTheta = std::abs(tanTheta(w));
    if (std::isinf(absTanTheta))
        return (0.);
    Float alpha = std::sqrt(cos2Phi(w) * alphaX * alphaX + sin2Phi(w) * alphaY * alphaY);
    Float alpha2Tan2Theta = (alpha * absTanTheta) * (alpha * absTanTheta);
    return ((-1 + std::sqrt(1.f + alpha2Tan2Theta)) / 2);
}

Vec3f TrowbridgeReitzDistribution::sampleWh(const Vec3f &wo, const Point2f &u) const
{
    Vec3f wh;
    if (!sampleVisibleArea)
    {
        Float cosTheta = 0, phi = (2 * PI) * u[1];
        if (alphaX == alphaY)
        {
            Float tanTheta2 = alphaX * alphaX * u[0] / (1.0f - u[0]);
            cosTheta = 1 / std::sqrt(1 + tanTheta2);
        }
        else
        {
            phi = std::atan(alphaY / alphaX * std::tan(2 * PI * u[1] + .5f * PI));
            if (u[1] > .5f)
                phi += PI;
            Float sinPhi = std::sin(phi), cosPhi = std::cos(phi);
            const Float alphax2 = alphaX * alphaX;
            const Float alphay2 = alphaY * alphaY;
            const Float alpha2 = 1 / (cosPhi * cosPhi / alphax2 + sinPhi * sinPhi / alphay2);
            Float tanTheta2 = alpha2 * u[0] / (1 - u[0]);
            cosTheta = 1 / std::sqrt(1 + tanTheta2);
        }
        Float sinTheta = std::sqrt(std::max((Float)0., (Float)1. - cosTheta * cosTheta));
        wh = sphericalDirection(sinTheta, cosTheta, phi);
        if (!sameHemisphere(wo, wh))
            wh = -wh;
    }
    else
    {
        bool flip = wo.z < 0;
        wh = trowbridgeReitzSample(flip ? -wo : wo, alphaX, alphaY, u[0], u[1]);
        if (flip)
            wh = -wh;
    }
    return wh;
}