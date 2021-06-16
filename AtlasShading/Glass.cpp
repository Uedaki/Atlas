#include "Glass.h"

#include "Material.h"

void atlas::Glass::evaluate(const Vec3f &wo, const SurfaceInteraction &si, const Point2f &sample, DataBlock &block) const
{
	BSDF bsdf = {};

	Float eta = iEta.get(block);
	bool entering = cosTheta(wo) > 0;
	Float etaI = entering ? 1.f : eta;
	Float etaT = entering ? eta : 1.f;
	if (refract(wo, faceForward(Normal(0, 0, 1), wo), etaI / etaT, bsdf.wi))
	{
		bsdf.pdf = 1;
		bsdf.scatteringPdf = 1;
		bsdf.Li = Spectrum(1.f);
	}
	else
	{
		bsdf.pdf = 0;
		bsdf.scatteringPdf = 0;
		bsdf.Le = Spectrum(0.f);
	}
	out.set(block, bsdf);
}

atlas::Spectrum atlas::Glass::f(const Vec3f &wo, const Vec3f &wi, const DataBlock &block) const
{
	return (Spectrum(0.f));
}

std::shared_ptr<atlas::Material> atlas::createGlassMaterial(Float eta)
{
	std::shared_ptr<Material> m = std::make_shared<Material>();
	Glass &glass = m->addShader<Glass>();
	m->bind(glass);

	auto &c = m->addShader<ConstantShader<Float>>();
	c.value = eta;
	glass.iEta.bind(c.out);

	return (m);
}