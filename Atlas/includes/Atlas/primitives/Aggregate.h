#pragma once

#include "atlas/core/Primitive.h"

namespace atlas
{
	class Aggregate : public Primitive
	{
	public:
		void computeScatteringFunctions(SurfaceInteraction &isect, TransportMode mode, bool allowMultipleLobes) const override {}
		const AreaLight *getAreaLight() const override { return (nullptr); }
		const Material *getMaterial() const override { return (nullptr); }
	};
}