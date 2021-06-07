#pragma once

#include "atlas/atlas.h"
#include "atlas/atlasLibHeader.h"
#include "atlas/atlasLibHeader.h"
#include "atlas/core/Transform.h"
#include "atlas/core/Shape.h"

namespace atlas
{
	enum class LightFlags : int
	{
		DeltaPosition = 1,
		DeltaDirection = 2,
		Area = 4,
		Infinite = 8
	};

	class VisibilityTester
	{
	public:
		VisibilityTester() = default;
		VisibilityTester(const Interaction &p0, const Interaction &p1)
			: p0(p0), p1(p1)
		{}

		ATLAS const Interaction &getP0() const { return p0; }
		ATLAS const Interaction &getP1() const { return p1; }

		ATLAS bool unoccluded(const Primitive &scene) const;

		ATLAS Spectrum tr(const Primitive &scene, Sampler &sampler) const;

	private:
		Interaction p0;
		Interaction p1;
	};

	class Light
	{
	public:
		Light(int flags, const Transform &lightToWorld, const MediumInterface &mediumInterface, int nSamples = 1)
			: flags(flags), nSamples(std::max(1, nSamples))
			, mediumInterface(mediumInterface)
			, lightToWorld(lightToWorld), worldToLight(lightToWorld.inverse())
		{

		}
		virtual ~Light() = default;

		virtual Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vec3f &wi, Float &pdf, VisibilityTester &vis) const = 0;
		virtual Spectrum power() const = 0;
		virtual Spectrum le(const RayDifferential &r) const;
		virtual Float pdfLi(const Interaction &ref, const Vec3f &wi) const = 0;
		virtual Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Float time, Ray &ray, Normal &nLight, Float &pdfPos, Float &pdfDir) const = 0;
		virtual void pdfLe(const Ray &ray, const Normal &nLight, Float &pdfPos, Float pdfDir) const = 0;

		const int flags;
		const int nSamples;
		const MediumInterface mediumInterface;

	protected:
		const Transform lightToWorld;
		const Transform worldToLight;
	};

	class AreaLight : public Light
	{
	public:
		AreaLight(const Transform &lightToWorld, const MediumInterface &mediumInterface, int nSamples)
			: Light((int)LightFlags::Area, lightToWorld, mediumInterface, nSamples)
		{}

		virtual Spectrum l(const Interaction &intr, const Vec3f &w) const = 0;
	};

	class DiffuseAreaLight : public AreaLight
	{
	public:
		DiffuseAreaLight(const Transform &lightToWorld, const MediumInterface &mediumInterface, const Spectrum &le, int nSamples, const std::shared_ptr<Shape> &shape)
			: AreaLight(lightToWorld, mediumInterface, nSamples)
			, lEmit(le), shape(shape), area(shape->area())
		{}

		Spectrum l(const Interaction &intr, const Vec3f &w) const override;
		Spectrum power() const override;
		Spectrum sampleLi(const Interaction &ref, const Point2f &u, Vec3f &wi, Float &pdf, VisibilityTester &vis) const override;
		Float pdfLi(const Interaction &ref, const Vec3f &wi) const override;
		Spectrum sampleLe(const Point2f &u1, const Point2f &u2, Float time, Ray &ray, Normal &nLight, Float &pdfPos, Float &pdfDir) const override;
		void pdfLe(const Ray &ray, const Normal &nLight, Float &pdfPos, Float pdfDir) const override;

	protected:
		const Spectrum lEmit;
		std::shared_ptr<Shape> shape;
		const Float area;
	};
}