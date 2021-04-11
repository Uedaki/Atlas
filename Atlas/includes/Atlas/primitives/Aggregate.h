#pragma once

#include "atlas/core/Primitive.h"

namespace atlas
{
	class Aggregate : public Primitive
	{
	public:
		void computeScatteringFunctions(SurfaceInteraction &isect, TransportMode mode, bool allowMultipleLobes) const override {}
		const AreaLight *getAreaLight() const override { return (nullptr); }
#if defined(SHADING)
		const sh::Material *getMaterial() const override { return (nullptr); }
#else
		const Material *getMaterial() const override { return (nullptr); }
#endif
	};
}