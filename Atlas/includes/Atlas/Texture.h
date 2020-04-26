#pragma once

namespace atlas
{
	namespace rendering { struct HitRecord; }

	class Texture
	{
	public:
		virtual glm::vec3 sample(const atlas::rendering::HitRecord &hit) const = 0;
	};
}