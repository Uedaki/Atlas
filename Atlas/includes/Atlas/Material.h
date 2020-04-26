#pragma once

#include <glm/glm.hpp>

namespace atlas
{
	namespace rendering { struct HitRecord; };
	namespace rendering { struct Ray; };

	class Material
	{
	public:
		virtual ~Material() = default;

		virtual bool scatter(const rendering::Ray &in, const rendering::HitRecord &hit, glm::vec3 &attenuation, rendering::Ray &scattered) const = 0;
		virtual glm::vec3 emitted(const rendering::HitRecord &hit) const { return (glm::vec3(0, 0, 0)); }
	};
}