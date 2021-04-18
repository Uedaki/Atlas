#include "Metal.h"

#include "Material.h"

void atlas::sh::Metal::evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const
{
	BSDF bsdf = {};
	bsdf.wi = Vec3f(-wo.x, -wo.y, wo.z);
	bsdf.pdf = 1;
	bsdf.Li = iR.get(block);
	out.set(block, bsdf);
}

atlas::Spectrum atlas::sh::Metal::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
{
	return (iR.get(block));
}

std::shared_ptr<atlas::sh::Material> atlas::sh::createMetalMaterial(const Spectrum &r)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Metal &metal = m->addShader<Metal>();
	auto &c = m->addShader<ConstantShader<Spectrum>>();
	c.value = r;
	metal.iR.bind(c.out);
	m->bind(metal);
	return (m);
}