#include "Materials.h"

#include "Atlas/rendering/HitRecord.h"
#include "Atlas/rendering/Ray.h"
#include "Atlas/Tools.h"

Lambert::Lambert(const glm::vec3 &a)
	: albedo(a)
{}

bool Lambert::scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const
{
	glm::vec3 dir = hit.normal + atlas::Tools::randomInUnitSphere() * 0.5f;
	scattered = atlas::rendering::Ray(hit.p, dir);
	attenuation = albedo;
	return (true);
}

Metal::Metal(const glm::vec3 &a, const float f)
	: albedo(a), fuzz(f)
{}

bool Metal::scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const
{
	glm::vec3 reflected = atlas::Tools::reflect(glm::normalize(in.dir), hit.normal);
	scattered = atlas::rendering::Ray(hit.p, reflected + fuzz * atlas::Tools::randomInUnitSphere());
	attenuation = albedo;
	return (glm::dot(scattered.dir, hit.normal) > 0);
}

Dielectric::Dielectric(float r)
	: ri(r)
{}

bool Dielectric::scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const
{
	glm::vec3 outwardNormal;
	glm::vec3 reflected = atlas::Tools::reflect(in.dir, hit.normal);
	float ni_over_nt;
	attenuation = glm::vec3(1, 1, 1);
	glm::vec3 refracted;
	float reflectProb;
	float cosine;
	if (glm::dot(in.dir, hit.normal) > 0)
	{
		outwardNormal = -hit.normal;
		ni_over_nt = ri;
		cosine = glm::dot(in.dir, hit.normal) / in.dir.length();
		//cosine = glm::sqrt(1 - _ri * _ri * (1 - cosine * cosine));
	}
	else
	{
		outwardNormal = hit.normal;
		ni_over_nt = 1 / ri;
		cosine = -glm::dot(in.dir, hit.normal) / in.dir.length();
	}
	if (atlas::Tools::refract(in.dir, outwardNormal, ni_over_nt, refracted))
		reflectProb = atlas::Tools::schlick(cosine, ri);
	else
		reflectProb = 1;
	if (atlas::Tools::rand() < reflectProb)
		scattered = atlas::rendering::Ray(hit.p, reflected);
	else
		scattered = atlas::rendering::Ray(hit.p, refracted);
	return (true);
}