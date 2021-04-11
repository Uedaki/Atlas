#include "Lambert.h"

#include "Material.h"

atlas::Spectrum atlas::sh::Lambert::f(const Vec3f &wo, const Vec3f &wi, const std::vector<uint8_t> &data) const
{
	return (r.getValue(data));
}

std::shared_ptr<atlas::sh::Material> atlas::sh::createLambertMaterial(const Spectrum &r)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Lambert &lambert = m->addShader<Lambert>();
	ConstantShader<Spectrum> &color = m->addShader<ConstantShader<Spectrum>>();
	color.value = r;
	lambert.r.connect(color.out);
	m->link(lambert);
	return (m);
}
