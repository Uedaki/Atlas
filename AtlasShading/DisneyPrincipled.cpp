#include "DisneyPrincipled.h"

#include "atlas/core/Math.h"

namespace
{
	Float schlickFresnel(Float u)
	{
		const float m = atlas::clamp(1 - u, 0, 1);
		const float m2 = m * m;
		return m2 * m2 * m; // pow(m,5)
	}
}

void atlas::DisneyPrincipled::evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const
{
	const Spectrum baseColor = iBaseColor.get(block);
	const Float subsurface = iSubsurface.get(block);
	const Float metallic = iMetallic.get(block);
	const Float specular = iSpecular.get(block);
	const Float specularTint = iSpecularTint.get(block);
	const Float roughness = iRoughness.get(block);
	const Float anisotropic = iAnisotropic.get(block);
	const Float sheen = iSheen.get(block);
	const Float sheenTint = iSheenTint.get(block);
	const Float clearcoat = iClearcoat.get(block);
	const Float clearcoatGloss = iClearcoatGloss.get(block);

	BSDF bsdf = {};

	Float rnd = random();
	if (rnd <= 1. / 3)
	{
		Onb uvw(si.n);
		bsdf.wi = uvw.local(cosineSampleDirection(sample));
		bsdf.pdf = sameHemisphere(wo, bsdf.wi, si.n) ? std::abs(dot(uvw.w, bsdf.wi)) * INV_PI : 0;
	}
	else if (rnd <= 2. / 3)
	{
		Float phi = 0;
		Float cosTheta = 0;

		Float aspect = std::sqrt(1. - anisotropic * 0.9);
		Float alphaX = std::max((Float)0.001, (roughness * roughness) / aspect);
		Float alphaY = std::max((Float).001, (roughness * roughness) * aspect);

		phi = atan(alphaY / alphaX * tan(2. * PI * sample.y + .5 * PI));
		if (sample.y > .5)
			phi += PI;

		Float sinPhi = sin(phi);
		Float cosPhi = cos(phi);
		Float alphaX2 = alphaX * alphaX;
		Float alphaY2 = alphaY * alphaY;
		Float alpha2 = 1. / (cosPhi * cosPhi / alphaX2 + sinPhi * sinPhi / alphaY2);
		Float tanTheta2 = alpha2 * sample.x / (1. - sample.x);
		cosTheta = 1. / std::sqrt(1. + tanTheta2);

		Float sinTheta = std::sqrt(std::max(0., 1. - cosTheta * cosTheta));
		Vec3f whLocal = sphericalDirection(sinTheta, cosTheta, phi);

		Onb uvw(si.n);
		Vec3f wh = uvw.local(whLocal);
		if (!sameHemisphere(wo, wh, si.n))
			wh *= -1;

		bsdf.wi = reflect(-wo, wh);
	}
	else
	{
		Float gloss = lerp(0.1, 0.001, clearcoatGloss);
		Float alpha2 = gloss * gloss;
		Float cosTheta = std::sqrt(std::max(0.0001, (1 - pow(alpha2, 1. - sample.x)) / (1. - alpha2)));
		Float sinTheta = std::sqrt(std::max(0.0001, 1. - cosTheta * cosTheta));
		Float phi = 2 * PI * sample.y;

		Vec3f whLocal = sphericalDirection(sinTheta, cosTheta, phi);

		Onb uvw(si.n);
		Vec3f wh = uvw.local(whLocal);
		if (!sameHemisphere(wo, wh, si.n))
			wh *= -1;

		bsdf.wi = reflect(-wo, wh);
	}




	bsdf.scatteringPdf = scatteringPdf(si, wo, bsdf.wi);
	bsdf.Li = f(wo, bsdf.wi, block);



	const Float nDotL = dot(si.n, wo);
	const Float nDotV = dot(si.n, bsdf.wi);

	const Vec3f h = normalize(wo + bsdf.wi);
	const Float nDotH = dot(si.n, h);
	const Float lDotH = dot(wo, h);

	Spectrum disneyDiffuse = diffuseModel(baseColor, roughness, nDotL, nDotV, lDotH);
	Spectrum disneySubsurface = subsurfaceModel(baseColor, roughness, nDotL, nDotV, lDotH);
	Spectrum disneyGlossy = microfacetAnisotropicModel(nDotL, nDot);
	Float disneyClearcoat;
	Spectrum disneySheen;

	bsdf.Li = (lerp(disneyDiffuse, disneySubsurface, subsurface) + disneySheen) * (1 - metallic) + disneyGlossy + disneyClearcoat;
	out.set(block, bsdf);
}

atlas::Spectrum atlas::DisneyPrincipled::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
{
	return (iBaseColor.get(block));
}

atlas::Spectrum atlas::DisneyPrincipled::diffuseModel(const Spectrum &baseColor, const Float roughness, const Float nDotL, const Float nDotV, const Float lDotH) const
{
	const Float fl = schlickFresnel(nDotL);
	const Float fv = schlickFresnel(nDotV);

	// Fd90 = 0.5 + 2 roughness * cos^2(Thetad)
	const Float fd90 = 0.5 + 2 * roughness * lDotH * lDotH;
	const Float fd = lerp((Float)1.0, fd90, fl) * lerp((Float)1.0, fd90, fv);
	return (baseColor * INV_PI * fd);
}

atlas::Spectrum atlas::DisneyPrincipled::subsurfaceModel(const Spectrum &baseColor, const Float roughness, const Float nDotL, const Float nDotV, const Float lDotH) const
{
	const Float fl = schlickFresnel(nDotL);
	const Float fv = schlickFresnel(nDotV);

	const Float fss90 = roughness * lDotH * lDotH;
	const Float fss = lerp((Float)1.0, fss90, fl) * lerp((Float)1.0, fss90, fv);
	const Float ss = 1.25 * (fss * (1. / (nDotL + nDotV) - .5) + .5);
	return (baseColor * INV_PI * ss);
}

atlas::Spectrum atlas::DisneyPrincipled::microfacetAnisotropicModel(Float nDotL, Float nDotV, Float nDotH, Float lDotH, const Vec3f &l, const Vec3 &v, const Vec3f &h, const Vec3f &x, const Vec3f &y)
{

}