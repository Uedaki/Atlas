#pragma once

#include "BoundingCone.h"

#include "../Atlas/includes/Atlas/rendering/Ray.h"
#include "HitRecord.h"

struct Cone
{
	uint32_t nbrRays;
	BoundingCone bound;
	atlas::rendering::Ray r[64];
	mutable HitRecord h[64];
	
	uint32_t nbrActive;
	bool actives[64];
};