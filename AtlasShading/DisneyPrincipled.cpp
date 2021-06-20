#include "DisneyPrincipled.h"

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
}

atlas::Spectrum atlas::DisneyPrincipled::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
{
	return (WHITE);
}