#pragma once

#include <vector>

#include "atlas/core/Block.h"
#include "atlas/core/Points.h"
#include "atlas/core/Vectors.h"
#include "atlas/core/Transform.h"

namespace atlas
{
	struct TriangleMesh
	{
		uint32_t nTriangle;
		uint32_t nVertices;
		std::vector<uint32_t> vertexIndices;
		std::unique_ptr<Point3f[]> p;
		std::unique_ptr<Normal[]> n;
		std::unique_ptr<Vec3f[]> s;
		std::unique_ptr<Point2f[]> uv;
		std::vector<uint32_t> faceIndices;

		TriangleMesh(const Transform &objectToWorld, uint32_t nTriangle, const uint32_t *vertexIndices, uint32_t nVertices, const Point3f *p, const Vec3f *s, const Normal *n, const Point2f *uv, const uint32_t faceIndices)
			: nTriangle(nTriangle), nVertices(nVertices)
			, vertexIndices(vertexIndices, vertexIndices + 3 * nTriangle)
		{
			this->p.reset(new Point3f[nVertices]);
			for (uint32_t i = 0; i < nVertices; i++)
				this->p[i] = objectToWorld(p[i]);

			if (uv)
			{
				this->uv.reset(new Point2f[nVertices]);
				memcpy(this->uv.get(), uv, nVertices * sizeof(Point2f));
			}

			if (n)
			{
				this->n.reset(new Normal[nVertices]);
				memcpy(this->n.get(), n, nVertices * sizeof(Normal));
			}

			if (s)
			{
				this->s.reset(new Vec3f[nVertices]);
				memcpy(this->s.get(), s, nVertices * sizeof(Vec3f));
			}

			if (faceIndices)
				this->faceIndices = std::vector<uint32_t>(faceIndices, faceIndices + nTriangle);
		}

		TriangleMesh(uint32_t nTriangle, const uint32_t *vertexIndices, uint32_t nVertices, const Point3f *p, const Vec3f *s, const Normal *n, const Point2f *uv, const uint32_t faceIndices)
			: nTriangle(nTriangle), nVertices(nVertices)
			, vertexIndices(vertexIndices, vertexIndices + 3 * nTriangle)
		{
			this->p.reset(new Point3f[nVertices]);
			for (uint32_t i = 0; i < nVertices; i++)
				this->p[i] = p[i];

			if (uv)
			{
				this->uv.reset(new Point2f[nVertices]);
				memcpy(this->uv.get(), uv, nVertices * sizeof(Point2f));
			}

			if (n)
			{
				this->n.reset(new Normal[nVertices]);
				memcpy(this->n.get(), n, nVertices * sizeof(Normal));
			}

			if (s)
			{
				this->s.reset(new Vec3f[nVertices]);
				memcpy(this->s.get(), s, nVertices * sizeof(Vec3f));
			}

			if (faceIndices)
				this->faceIndices = std::vector<uint32_t>(faceIndices, faceIndices + nTriangle);
		}
	};
}