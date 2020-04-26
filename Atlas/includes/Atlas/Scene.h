#pragma once

#include <memory>
#include <vector>

#include "Atlas/Material.h"
#include "Atlas/Shape.h"

namespace atlas
{
	class Scene
	{
	public:
		template <typename ShapeType, typename ...Args>
		ShapeType &defineShape(Args... args)
		{
			shapes.emplace_back(std::make_unique<ShapeType>(args...));
			return (*dynamic_cast<ShapeType *>(shapes.back().get()));
		}

		template <typename MaterialType, typename ...Args>
		MaterialType &defineMaterial(Args... args)
		{
			materials.emplace_back(std::make_unique<MaterialType>(args...));
			return (*dynamic_cast<MaterialType *>(materials.back().get()));
		}

		inline std::vector<std::unique_ptr<Shape>> &getShapes() { return (shapes); }
		inline const std::vector<std::unique_ptr<Shape>> &getShapes() const { return (shapes); }
		inline std::vector<std::unique_ptr<Material>> &getMaterials() { return (materials); }
		inline const std::vector<std::unique_ptr<Material>> &getMaterials() const { return (materials); }

	private:
		std::vector<std::unique_ptr<Shape>> shapes;
		std::vector<std::unique_ptr<Material>> materials;
	};
}