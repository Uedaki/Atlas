#pragma once

#include "Atlas/MaterialInput.h"
#include "atlas/Texture.h"
#include "Atlas/rendering/HitRecord.h"

class CheckerTexture : public atlas::Texture
{
public:
	CheckerTexture() = default;
	CheckerTexture(const atlas::MaterialInput &a, const atlas::MaterialInput &b) : a(a), b(b) {}

	glm::vec3 sample(const atlas::rendering::HitRecord &hit) const override
	{
		float sines = sin(10.f * hit.p.x) * sin(10.f * hit.p.y) * sin(10.f * hit.p.z);
		if (0 < sines)
			return (a.sample(hit));
		else
			return (b.sample(hit));
	}

private:
	atlas::MaterialInput a;
	atlas::MaterialInput b;
};