#include "pch.h"
#include "Camera.h"

#include <glm/gtc/constants.hpp>

#include <stdexcept>

#include "Rendering/Ray.h"
#include "Tools.h"

atlas::Camera::Camera(const glm::vec3 &p, const glm::vec3 &t, const glm::vec3 &h, float f, float a)
	: pos(p), target(t), up(h)
	, fov(f), aspect(a)
	, isBlurEnable(false), aperture(0), focusDistance(0)
	, u(0), v(0), w(0)
	, corner(0), horizontal(0), vertical(0)
{
	reset();
}

void atlas::Camera::enableDefocusBlur(float a, float f)
{
	isBlurEnable = true;
	aperture = a;
	focusDistance = f;
	lensRadius = aperture / 2;

	float theta = fov * glm::pi<float>() / 180;
	float halfHeight = glm::tan(theta / 2);
	float halfWidth = aspect * halfHeight;

	w = glm::normalize(pos - target);
	u = glm::normalize(glm::cross(up, w));
	v = glm::cross(w, u);

	corner = pos - halfWidth * u * focusDistance - halfHeight * v * focusDistance - w * focusDistance;
	horizontal = 2 * halfWidth * u * focusDistance;
	vertical = 2 * halfHeight * v * focusDistance;
}

void atlas::Camera::reset()
{
	isBlurEnable = false;

	float theta = fov * glm::pi<float>() / 180;
	float halfHeight = glm::tan(theta / 2);
	float halfWidth = aspect * halfHeight;

	w = glm::normalize(pos - target);
	u = glm::normalize(glm::cross(up, w));
	v = glm::cross(w, u);

	corner = pos - halfWidth * u - halfHeight * v - w;
	horizontal = 2 * halfWidth * u;
	vertical = 2 * halfHeight * v;
}

atlas::rendering::Ray atlas::Camera::getRay(const float x, const float y) const
{
	if (isBlurEnable)
	{
		glm::vec3 rd = lensRadius * Tools::randomInUnitDisk();
		glm::vec3 offset = u * rd.x + v * rd.y;
		return (rendering::Ray(pos + offset, corner + x * horizontal + y * vertical - pos - offset));
	}
	else
	{
		return (rendering::Ray(pos, corner + x * horizontal + y * vertical - pos));
	}
}

SimdRay atlas::Camera::getSimdRay(const float4 x, const float4 y) const
{
	if (isBlurEnable)
	{
		//glm::vec3 rd = lensRadius * Tools::randomInUnitDisk();
		//glm::vec3 offset = u * rd.x + v * rd.y;
		//return (rendering::Ray(pos + offset, corner + x * horizontal + y * vertical - pos - offset));
		throw std::runtime_error("Not implemented");
	}
	else
	{
		float4 cornerX(corner.x);
		float4 cornerY(corner.y);
		float4 cornerZ(corner.z);

		float4 hX(horizontal.x);
		float4 hY(horizontal.y);
		float4 hZ(horizontal.z);

		float4 vX(vertical.x);
		float4 vY(vertical.y);
		float4 vZ(vertical.z);

		float4 posX(pos.x);
		float4 posY(pos.y);
		float4 posZ(pos.z);

		float4 dirX = cornerX + x * hX + y * vX - posX;
		float4 dirY = cornerY + x * hY + y * vY - posY;
		float4 dirZ = cornerZ + x * hZ + y * vZ - posZ;

		return (SimdRay(posX, posY, posZ, dirX, dirY, dirZ));
	}
}