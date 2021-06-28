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

	Float smithG_GGX(Float NdotV, Float alphaG)
	{
		Float a = alphaG * alphaG;
		Float b = NdotV * NdotV;
		return 1 / (NdotV + sqrt(a + b - a * b));
	}

	Float smithG_GGX_aniso(Float NdotV, Float VdotX, Float VdotY, Float ax, Float ay)
	{
		return 1 / (NdotV + sqrt(std::pow(VdotX * ax, 2) + std::pow(VdotY * ay, 2) + pow(NdotV, 2)));
	}

	Float GTR2_aniso(Float NdotH, Float HdotX, Float HdotY, Float ax, Float ay)
	{
		return 1 / (PI * ax * ay * std::pow(std::pow(HdotX / ax, 2) + std::pow(HdotY / ay, 2) + NdotH * NdotH, 2));
	}

	Float GTR1(Float NdotH, Float a)
	{
		if (a >= 1)
			return 1 / PI;
		Float a2 = a * a;
		Float t = 1 + (a2 - 1) * NdotH * NdotH;
		return (a2 - 1) / (PI * log(a2) * t);
	}
}

void atlas::DisneyPrincipled::evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const
{
	const Float metallic = iMetallic.get(block);
	const Float roughness = iRoughness.get(block);
	const Float anisotropic = iAnisotropic.get(block);
	const Float clearcoatGloss = iClearcoatGloss.get(block);

	BSDF bsdf = {};

	Float rnd = random();
	if (rnd < metallic)
	{
		Float newRnd = rnd * 1 / metallic;
		if (newRnd)
		{
			// Specular
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

			{
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

			{
				Vec3f wh = normalize(wo + bsdf.wi);
				float hDotX = dot(wh, si.dpdx);
				float hDotY = dot(wh, si.dpdy);
				float ndotH = dot(si.n, wh);

				float denom = hDotX * hDotX / alphaX2 + hDotY * hDotY / alphaY2 + ndotH * ndotH;
				if (denom == 0.)
					bsdf.pdf = 0;
				else
				{
					Float pdfDistribution = ndotH / (PI * alphaX * alphaY * denom * denom);
					bsdf.pdf = pdfDistribution / (4. * dot(wo, wh));
				}
			}
		}
		else
		{
			// Clear coat
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

			{
				Vec3f wh = normalize(wo + bsdf.wi);
				float ndotH = dot(si.n, wh);

				float Dr = GTR1(ndotH, lerp(.1, .001, clearcoatGloss));
				bsdf.pdf = Dr * ndotH / (4. * dot(wo, wh));
			}
		}
	}
	else
	{
		// Lambertian
		Onb uvw(si.n);
		bsdf.wi = uvw.local(cosineSampleDirection(sample));
		bsdf.pdf = sameHemisphere(wo, bsdf.wi, si.n) ? std::abs(dot(uvw.w, bsdf.wi)) * INV_PI : 0;
	}

	const Float nDotL = dot(si.n, wo);
	const Float nDotV = dot(si.n, bsdf.wi);

	const Vec3f h = normalize(wo + bsdf.wi);
	const Float nDotH = dot(si.n, h);
	const Float lDotH = dot(wo, h);

	Spectrum diffuse = diffuseModel(block, nDotL, nDotV, lDotH);
	Spectrum specular = specularModel(block, wo, bsdf.wi, si.dpdx, si.dpdy, h, nDotL, nDotV, nDotH, lDotH);
	
	bsdf.Li = diffuse * (1 - metallic) + specular;
	bsdf.scatteringPdf = scatteringPdf(si, wo, bsdf.wi);
	out.set(block, bsdf);
}

atlas::Spectrum atlas::DisneyPrincipled::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
{
	return (iBaseColor.get(block));
}

atlas::Spectrum atlas::DisneyPrincipled::diffuseModel(const DataBlock &block, const Float nDotL, const Float nDotV, const Float lDotH) const
{
	const Spectrum baseColor = iBaseColor.get(block);
	const Float subsurface = iSubsurface.get(block);
	const Float roughness = iRoughness.get(block);
	const Float sheen = iSheen.get(block);
	const Float sheenTint = iSheenTint.get(block);

	const Float fl = schlickFresnel(nDotL);
	const Float fv = schlickFresnel(nDotV);

	// Base diffuse
	const Float fd90 = 0.5 + 2 * roughness * lDotH * lDotH;
	const Float fd = lerp((Float)1.0, fd90, fl) * lerp((Float)1.0, fd90, fv);
	const Spectrum baseDiffuse = baseColor * INV_PI * fd;

	// Subsurface
	const Float fss90 = roughness * lDotH * lDotH;
	const Float fss = lerp((Float)1.0, fss90, fl) * lerp((Float)1.0, fss90, fv);
	const Float ss = 1.25 * (fss * (1. / (nDotL + nDotV) - .5) + .5);
	const Spectrum subsurfaceDiffuse = ss * baseColor;

	// Sheen
	const Spectrum sheenColor = lerp(Spectrum(1), baseColor / luminance(baseColor), sheenTint);
	const Spectrum sheenDiffuse = sheen * schlickFresnel(lDotH) * sheenColor;

	return (lerp(baseDiffuse, subsurfaceDiffuse, subsurface) + sheenDiffuse);
}

atlas::Spectrum atlas::DisneyPrincipled::specularModel(const DataBlock &block, const Vec3f &wo, const Vec3f &wi, const Vec3f &tangent, const Vec3f &binormal, const Vec3f &h, const Float nDotL, const Float nDotV, const Float nDotH, const Float lDotH) const
{
	const Spectrum baseColor = iBaseColor.get(block);
	const Float metallic = iMetallic.get(block);
	const Float specular = iSpecular.get(block);
	const Float specularTint = iSpecularTint.get(block);
	const Float roughness = iRoughness.get(block);
	const Float anisotropic = iAnisotropic.get(block);
	const Float clearcoat = iClearcoat.get(block);
	const Float clearcoatGloss = iClearcoatGloss.get(block);

	Float aspect = std::sqrt(1 - anisotropic * .9);
	Float ax = std::max(.001, std::pow(roughness, 2) / aspect);
	Float ay = std::max(.001, std::pow(roughness, 2) * aspect);

	// Specular D
	const Float specularD = GTR2_aniso(nDotH, dot(h, tangent), dot(h, binormal), ax, ay);

	// Specular F
	const Spectrum color = lerp(specular * 0.8 * lerp(WHITE, baseColor, specularTint), baseColor, metallic);
	const Spectrum specularF = color + (WHITE - color) * schlickFresnel(lDotH);
	
	// Specular G
	const Float specularG = smithG_GGX_aniso(nDotL, dot(wo, tangent), dot(wo, binormal), ax, ay) * smithG_GGX_aniso(nDotV, dot(wi, tangent), dot(wi, binormal), ax, ay);

	// Clear coat D
	const Float clearCoatD = GTR1(nDotH, lerp(0.1, 0.001, clearcoatGloss));

	// Clear coat F
	const Float clearCoatF = lerp(0.4, 1.0, schlickFresnel(lDotH));
	
	// Clear coat G
	const Float clearCoatG = smithG_GGX(nDotL, 0.25) * smithG_GGX(nDotV, 0.25);

	return (specularG * specularF * specularD + 0.25 * clearcoat * clearCoatD * clearCoatF * clearCoatG);
}