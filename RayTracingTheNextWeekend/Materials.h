#pragma once

#include <glm/glm.hpp>

#include "Atlas/Material.h"
#include "Atlas/MaterialInput.h"

class Lambert : public atlas::Material
{
	atlas::MaterialInput albedo;

public:
	Lambert(const atlas::MaterialInput &albedo);

	bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;
};

class Metal : public atlas::Material
{
	atlas::MaterialInput albedo;
	float fuzz;

public:
	Metal(const atlas::MaterialInput &albedo, const float fuzz);

	bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;
};

class Dielectric : public atlas::Material
{
	float ri;

public:
	Dielectric(const float ri);

	bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;
};

class Light : public atlas::Material
{
	atlas::MaterialInput emit;

	public:
		Light(atlas::MaterialInput &a);

		bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;

		glm::vec3 emitted(const atlas::rendering::HitRecord &hit) const override;
};