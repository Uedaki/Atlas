#include "atlas/core/Reflection.h"

using namespace atlas;

Float atlas::fresnelDielectric(Float cosThetaI, Float etaI, Float etaT)
{
	cosThetaI = clamp(cosThetaI, -1.f, 1.f);
	bool entering = cosThetaI > 0.f;
	if (!entering)
	{
		std::swap(etaI, etaT);
		cosThetaI = std::abs(cosThetaI);
	}

	Float sinThetaI = std::sqrt(std::max(0.f, 1 - cosThetaI * cosThetaI));
	Float sinThetaT = etaI / etaT * sinThetaI;
	if (sinThetaT >= 1)
		return (1);
	Float cosThetaT = std::sqrt(std::max(0.f, 1 - sinThetaT * sinThetaT));

	Float rParl = ((etaT * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaI * cosThetaT));
	Float rPerp = ((etaI * cosThetaI) - (etaI * cosThetaT)) / ((etaT * cosThetaI) + (etaT * cosThetaT));
	return ((rParl * rParl + rPerp * rPerp) / 2);
}

Spectrum atlas::fresnelConductor(Float cosThetaI, const Spectrum &etaI, const Spectrum &etaT, const Spectrum &k)
{
	cosThetaI = clamp(cosThetaI, -1.f, 1.f);
	Spectrum eta = etaT / etaI;
	Spectrum etak = k / etaI;

	Float cosThetaI2 = cosThetaI * cosThetaI;
	Float sinThetaI2 = 1. - cosThetaI2;
	Spectrum eta2 = eta * eta;
	Spectrum etak2 = etak * etak;

	Spectrum t0 = eta2 - etak2 - sinThetaI2;
	Spectrum a2plusb2 = sqrt(t0 * t0 + 4 * eta2 * etak2);
	Spectrum t1 = a2plusb2 + cosThetaI2;
	Spectrum a = sqrt(0.5f * (a2plusb2 + t0));
	Spectrum t2 = (Float)2 * cosThetaI * a;
	Spectrum Rs = (t1 - t2) / (t1 + t2);

	Spectrum t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
	Spectrum t4 = t2 * sinThetaI2;
	Spectrum Rp = Rs * (t3 - t4) / (t3 + t4);

	return 0.5 * (Rp + Rs);
}