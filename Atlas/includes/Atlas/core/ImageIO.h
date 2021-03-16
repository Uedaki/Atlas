#pragma once

#include <string>
#include <fstream>

#include "atlas/Atlas.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/Points.h"

namespace atlas
{
	inline void writeImageToFile(const std::string &name, const Float *rgb,
		const Bounds2i &outputBounds, const Point2i &totalResolution)
	{
		std::ofstream file(name);
		if (file)
		{
			file << "P3\n"
				<< totalResolution.x << " " << totalResolution.y << "\n"
				<< "255\n";

			for (int32_t j = 0; j < totalResolution.y; ++j) // this loop check for j < _ny because when as j is unsigned -1 will be the highest value j can hold, therefore higher than _ny
			{
				for (int32_t i = 0; i < totalResolution.x; ++i)
				{
					int r = static_cast<int>(255.99 * rgb[3 * (j * totalResolution.x + i)]);
					int g = static_cast<int>(255.99 * rgb[3 * (j * totalResolution.x + i) + 1]);
					int b = static_cast<int>(255.99 * rgb[3 * (j * totalResolution.x + i) + 2]);
					file << r << " " << g << " " << b << "\n";
				}
			}
			file.close();
		}
	}
}