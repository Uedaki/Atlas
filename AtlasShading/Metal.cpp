#include "Metal.h"

#include "Material.h"

void atlas::sh::Metal::evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const
{
	BSDF bsdf;
	bsdf.wi = Vec3f(-wo.x, -wo.y, wo.z);
	bsdf.pdf = 1;
	bsdf.color = r;
	out.set(data, bsdf);
}

atlas::Spectrum atlas::sh::Metal::f(const Vec3f &wo, const Vec3f &wi, const std::vector<uint8_t> &data) const
{
	return (r);
}

std::shared_ptr<atlas::sh::Material> atlas::sh::createMetalMaterial(const Spectrum &r)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Metal &metal = m->addShader<Metal>();
	metal.r = r;
	m->link(metal);
	return (m);
}