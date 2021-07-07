#include "Lambert.h"

#include "Material.h"

atlas::Spectrum atlas::Lambert::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &data) const
{
	return (iR.get(data));
}

std::shared_ptr<atlas::Material> atlas::createLambertMaterial(const Spectrum &r)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Lambert &lambert = m->addEntryShader<Lambert>();
	ConstantShader<Spectrum> &color = m->addConstant<Spectrum>(r);
	lambert.iR.bind(color.out);
	return (m);
}
