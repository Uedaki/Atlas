#pragma once

#include <glm/glm.hpp>

#include <memory>

#include "Atlas/Texture.h"

namespace atlas
{
	class MaterialInput
	{
	public:
		MaterialInput() = default;
		MaterialInput(float r, float g, float b) : color(r, g, b) {};
		MaterialInput(std::shared_ptr<Texture> newTexture) : texture(newTexture) {}

		inline glm::vec3 sample(const atlas::rendering::HitRecord &hit) const { return (texture ? texture->sample(hit) : color); }

	private:
		glm::vec3 color = glm::vec3(0, 0, 0);
		std::shared_ptr<Texture> texture;
	};
}