#include "Lambert.h"

#include "Material.h"

atlas::Spectrum atlas::Lambert::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &data) const
{
	return (iR.get(data));
}

std::shared_ptr<atlas::Material> atlas::createLambertMaterial(const Spectrum &r)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Lambert &lambert = m->addShader<Lambert>();
	ConstantShader<Spectrum> &color = m->addShader<ConstantShader<Spectrum>>();
	color.value = r;
	lambert.iR.bind(color.out);
	m->bind(lambert);
	return (m);
}
