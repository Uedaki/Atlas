#pragma once

#include <cstdint>
#include <vector>

#include "atlas/Atlas.h"
#include "atlas/core/RgbSpectrum.h"
#include "atlas/core/Points.h"

#include "AtlasShadingLibHeader.h"
#include "stb_image.h"

namespace atlas
{
	namespace sh
	{
		typedef size_t ImageID;

		class ImageWrapper
		{
		public:
			ImageWrapper(const std::string &filename)
				: filename(filename), buffer(nullptr)
				, width(0), height(0), lastCallValue(0)
				, bIsLoaded(false), bHasAlpha(false)
			{}

			ATLAS_SH bool load();
			ATLAS_SH void unload();
			ATLAS_SH Spectrum sample(const Point2f &uv, const uint32_t callValue, Float &alpha);

			inline const std::string &getFilename() const
			{
				return (filename);
			}

			inline uint32_t getLastCallValue() const
			{
				return (lastCallValue);
			}

			inline bool isLoaded() const
			{
				return (bIsLoaded);
			}

		private:
			std::string filename;
			uint8_t *buffer;
			int32_t width;
			int32_t height;
			uint32_t lastCallValue;
			bool bIsLoaded;
			bool bHasAlpha;
		};

		class ImageLibrary
		{
		public:
			inline static ImageID requestID(const std::string &filename)
			{
				return (instance.privateRequestID(filename));
			}

			inline static Spectrum sample(const ImageID id, const Point2f &uv, Float &alpha)
			{
				return (instance.privateSample(id, uv, alpha));
			}

		private:
			ATLAS_SH static ImageLibrary instance;
			uint32_t nextCallValue = 0;
			std::vector<ImageWrapper> images;

			ATLAS_SH ImageID privateRequestID(const std::string &filename);
			ATLAS_SH Spectrum privateSample(const ImageID id, const Point2f &uv, Float &alpha);
		};
	}
}