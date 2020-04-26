#include "pch.h"
#include "Tools.h"

#include <glm/gtc/constants.hpp>

float atlas::Tools::rand()
{
	thread_local static uint32_t x = 123456789;
	thread_local static uint32_t y = 362436069;
	thread_local static uint32_t z = 521288629;
	thread_local static uint32_t w = 88675123;
	uint32_t t;
	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ (t ^ (t >> 8));
	return (static_cast<float>(w & 0xffffff) / 16777216.0f);
}

glm::vec3 atlas::Tools::randomInUnitSphere()
{
	glm::vec3 p;
	do {
		p = 2.0f * glm::vec3(rand(),
			rand(),
			rand()) - glm::vec3(1, 1, 1);
	} while (glm::length(p) >= 1);
	return (p);
}

glm::vec3 atlas::Tools::randomUniformHemisphere()
{
	const float u1 = 2 * rand() - 1;
	const float u2 = rand();

	const float r = sqrt(1.f - u1 * u1);
	const float phi = 2 * glm::pi<float>() * u2;

	return (glm::vec3(cos(phi) * r, sin(phi) * r, u1));
}

glm::vec3 atlas::Tools::randomCosineHemisphere()
{
	float u1 = rand();
	float u2 = rand();

	const float r = sqrt(u1);
	const float theta = 2 * glm::pi<float>() * u2;

	const float x = r * cos(theta);
	const float y = r * sin(theta);

	return (glm::vec3(x, y, sqrt(0.f > 1 - u1 ? 0.f : 1 - u1)));
}

bool atlas::Tools::refract(const glm::vec3 &v, const glm::vec3 &n, const float ni_over_nt, glm::vec3 &refracted)
{
	glm::vec3 uv = glm::normalize(v);
	float dt = glm::dot(uv, n);
	float discriminant = 1 - ni_over_nt * ni_over_nt * (1 - dt * dt);
	if (discriminant > 0)
	{
		refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
		return (true);
	}
	return (false);
}

float atlas::Tools::schlick(const float cosine, const float ri)
{
	float r0 = (1 - ri) / (1 + ri);
	r0 = r0 * r0;
	return (r0 + (1 - r0) * pow(1 - cosine, 5));
}

glm::vec3 atlas::Tools::reflect(const glm::vec3 &v, const glm::vec3 &n)
{
	return (v - 2 * glm::dot(v, n) * n);
}

glm::vec3 atlas::Tools::randomInUnitDisk()
{
	glm::vec3 p;
	do
	{
		p = 2.f * glm::vec3(rand(), rand(), 0) - glm::vec3(1, 1, 0);
	} while (glm::dot(p, p) >= 1);
	return (p);
}