#pragma once

#include <glm/glm.hpp>

#include "Atlas/Material.h"

class Lambert : public atlas::Material
{
	glm::vec3 albedo;

public:
	Lambert(const glm::vec3 &albedo);

	bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;
};

class Metal : public atlas::Material
{
	glm::vec3 albedo;
	float fuzz;

public:
	Metal(const glm::vec3 &albedo, const float fuzz);

	bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;
};

class Dielectric : public atlas::Material
{
	float ri;

public:
	Dielectric(const float ri);

	bool scatter(const atlas::rendering::Ray &in, const atlas::rendering::HitRecord &hit, glm::vec3 &attenuation, atlas::rendering::Ray &scattered) const override;
};