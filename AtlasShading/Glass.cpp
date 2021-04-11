#include "Glass.h"

#include "Material.h"

void atlas::sh::Glass::evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, std::vector<uint8_t> &data) const
{
	BSDF bsdf;

	bool entering = cosTheta(wo) > 0;
	Float etaI = entering ? 1.f : eta;
	Float etaT = entering ? eta : 1.f;
	if (refract(wo, faceForward(Normal(0, 0, 1), wo), etaI / etaT, bsdf.wi))
	{
		bsdf.pdf = 1;
		bsdf.color = Spectrum(1.f);
	}
	else
	{
		bsdf.pdf = 0;
		bsdf.color = Spectrum(0.f);
	}
	out.set(data, bsdf);
}

atlas::Spectrum atlas::sh::Glass::f(const Vec3f &wo, const Vec3f &wi, const std::vector<uint8_t> &data) const
{
	return (Spectrum(0.f));
}

std::shared_ptr<atlas::sh::Material> atlas::sh::createGlassMaterial(Float eta)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Glass &glass = m->addShader<Glass>();
	glass.eta = eta;
	m->link(glass);
	return (m);
}