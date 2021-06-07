#pragma once

#include <cstdint>

#include "atlas/Atlas.h"
#include "atlas/core/Bounds.h"
#include "atlas/core/Geometry.h"
#include "atlas/core/Interaction.h"
#include "atlas/core/Matrix4x4.h"
#include "atlas/core/Points.h"
#include "atlas/core/Ray.h"
#include "atlas/core/Vectors.h"

namespace atlas
{
	inline bool solveLinearSystem2x2(const Float A[2][2], const Float B[2], Float &x0, Float &x1)
	{
		Float det = A[0][0] * A[1][1] - A[0][1] * A[1][0];
		if (std::abs(det) < 1e-10f)
			return false;
		x0 = (A[1][1] * B[0] - A[0][1] * B[1]) / det;
		x1 = (A[0][0] * B[1] - A[1][0] * B[0]) / det;
		if (std::isnan(x0) || std::isnan(x1))
			return false;
		return true;
	}

	class Transform
	{
	public:
		Transform() = default;
		Transform(const Float mat[4][4])
			: m(mat[0][0], mat[0][1], mat[0][2], mat[0][3],
				mat[1][0], mat[1][1], mat[1][2], mat[1][3],
				mat[2][0], mat[2][1], mat[2][2], mat[2][3],
				mat[3][0], mat[3][1], mat[3][2], mat[3][3])
			, mInv(m.inverse())
		{}
		Transform(const Matrix4x4 &m)
			: m(m)
			, mInv(m.inverse())
		{}

		Transform(const Matrix4x4 &m, const Matrix4x4 &mInv)
			: m(m), mInv(mInv)
		{}

		static Transform identity()
		{
			Matrix4x4 m(1, 0, 0, 0,
				0, 1, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);
			return (Transform(m));
		}

		static Transform translate(const Vec3f &delta)
		{
			Matrix4x4 m(1, 0, 0, delta.x,
				0, 1, 0, delta.y,
				0, 0, 1, delta.z,
				0, 0, 0, 1);
			return (Transform(m));
		}

		static Transform scale(Float x, Float y, Float z)
		{
			Matrix4x4 m(x, 0, 0, 0,
				0, y, 0, 0,
				0, 0, z, 0,
				0, 0, 0, 1);
			return (Transform(m));
		}

		static Transform rotateX(Float theta)
		{
			Float sinTheta = std::sin(radians(theta));
			Float cosTheta = std::cos(radians(theta));
			Matrix4x4 m(1, 0, 0, 0,
				0, cosTheta, -sinTheta, 0,
				0, sinTheta, cosTheta, 0,
				0, 0, 0, 1);
			return (Transform(m));
		}

		static Transform rotateY(Float theta)
		{
			Float sinTheta = std::sin(radians(theta));
			Float cosTheta = std::cos(radians(theta));
			Matrix4x4 m(cosTheta, 0, sinTheta, 0,
				0, 1, 0, 0,
				-sinTheta, 0, cosTheta, 0,
				0, 0, 0, 1);
			return (Transform(m));
		}

		static Transform rotateZ(Float theta)
		{
			Float sinTheta = std::sin(radians(theta));
			Float cosTheta = std::cos(radians(theta));
			Matrix4x4 m(cosTheta, -sinTheta, 0, 0,
				sinTheta, cosTheta, 0, 0,
				0, 0, 1, 0,
				0, 0, 0, 1);
			return (Transform(m));
		}

		static Transform rotate(Float theta, const Vec3f &axis)
		{
			Vec3f a = normalize(axis);
			Float sinTheta = std::sin(radians(theta));
			Float cosTheta = std::cos(radians(theta));

			Matrix4x4 m;
			m.m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
			m.m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
			m.m[0][2] = a.x * a.z + (1 - cosTheta) - a.y * sinTheta;
			m.m[0][3] = 0;

			m.m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
			m.m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
			m.m[1][2] = a.y * a.z + (1 - cosTheta) - a.x * sinTheta;
			m.m[1][3] = 0;

			m.m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
			m.m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
			m.m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
			m.m[2][3] = 0;

			m.m[3][0] = m.m[3][1] = m.m[3][2] = m.m[3][3] = 0.f;
			return (Transform(m));
		}

		static Transform lookAt(const Point3f &pos, const Point3f &look, const Vec3f &up)
		{
			Matrix4x4 cameraToWorld;

			cameraToWorld.m[0][3] = pos.x;
			cameraToWorld.m[1][3] = pos.y;
			cameraToWorld.m[2][3] = pos.z;
			cameraToWorld.m[3][3] = 1;

			Vec3f dir = normalize(look - pos);
			if (cross(normalize(up), dir).length() == 0)
			{
				return Transform();
			}
			Vec3f right = normalize(cross(normalize(up), dir));
			Vec3f newUp = cross(dir, right);
			cameraToWorld.m[0][0] = right.x;
			cameraToWorld.m[1][0] = right.y;
			cameraToWorld.m[2][0] = right.z;
			cameraToWorld.m[3][0] = 0.;
			cameraToWorld.m[0][1] = newUp.x;
			cameraToWorld.m[1][1] = newUp.y;
			cameraToWorld.m[2][1] = newUp.z;
			cameraToWorld.m[3][1] = 0.;
			cameraToWorld.m[0][2] = dir.x;
			cameraToWorld.m[1][2] = dir.y;
			cameraToWorld.m[2][2] = dir.z;
			cameraToWorld.m[3][2] = 0.;
			return Transform(cameraToWorld.inverse(), cameraToWorld);
		}

		Transform inverse() const
		{
			return (Transform(m.inverse()));
		}

		Transform transpose() const
		{
			return (Transform(m.transpose()));
		}

		bool swapsHandedness() const
		{
			Float det = m.m[0][0] * (m.m[1][1] * m.m[2][2] - m.m[1][2] * m.m[2][1]) -
				m.m[0][1] * (m.m[1][0] * m.m[2][2] - m.m[1][2] * m.m[2][0]) +
				m.m[0][2] * (m.m[1][0] * m.m[2][1] - m.m[1][1] * m.m[2][0]);
			return det < 0;
		}

		bool operator==(const Transform &t) const
		{
			return (m == t.m);
		}

		bool operator!=(const Transform &t) const
		{
			return (m != t.m);
		}

		bool operator<(const Transform &t2) const
		{
			for (int8_t i = 0; i < 4; i++)
			{
				for (int8_t j = 0; j < 4; j++)
				{
					if (m.m[i][j] < t2.m.m[i][j])
						return (true);
					else if (m.m[i][j] > t2.m.m[i][j])
						return (false);
				}
				return (false);
			}
		}

		bool operator>(const Transform &t2) const
		{
			for (int8_t i = 0; i < 4; i++)
			{
				for (int8_t j = 0; j < 4; j++)
				{
					if (m.m[i][j] < t2.m.m[i][j])
						return (false);
					else if (m.m[i][j] > t2.m.m[i][j])
						return (true);
				}
				return (false);
			}
		}

		bool isIdentity() const
		{
			return (m.m[0][0] == 1.f && m.m[0][1] == 0.f && m.m[0][2] == 0.f && m.m[0][3] == 0.f
				&& m.m[1][0] == 0.f && m.m[1][1] == 1.f && m.m[1][2] == 0.f && m.m[1][3] == 0.f
				&& m.m[2][0] == 0.f && m.m[2][1] == 0.f && m.m[2][2] == 1.f && m.m[2][3] == 0.f
				&& m.m[3][0] == 0.f && m.m[3][1] == 0.f && m.m[3][2] == 0.f && m.m[3][3] == 1.f);
		}

		const Matrix4x4 &getMatrix() const { return (m); }

		bool hasScale() const
		{
			Float la2 = (*this)(Vec3f(1, 0, 0)).lengthSquared();
			Float lb2 = (*this)(Vec3f(0, 1, 0)).lengthSquared();
			Float lc2 = (*this)(Vec3f(0, 0, 1)).lengthSquared();
			return (0.999f > la2 || la2 > 1.001f
				|| 0.999f > lb2 || lb2 > 1.001f
				|| 0.999f > lc2 || lc2 > 1.001f);
		}

		template <typename T>
		inline Point3<T> operator()(const Point3<T> &p) const
		{
			T x = p.x, y = p.y, z = p.z;
			T xp = m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z + m.m[0][3];
			T yp = m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z + m.m[1][3];
			T zp = m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z + m.m[2][3];
			T wp = m.m[3][0] * x + m.m[3][1] * y + m.m[3][2] * z + m.m[3][3];
			if (wp == 1)
				return Point3<T>(xp, yp, zp);
			else
				return Point3<T>(xp, yp, zp) / wp;
		}

		template <typename T>
		inline Vector3<T> operator()(const Vector3<T> &v) const
		{
			T x = v.x, y = v.y, z = v.z;
			return Vector3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
				m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
				m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
		}

		template <typename T>
		inline Point3<T> operator()(const Point3<T> &p, Vector3<T> &pError) const
		{
			T x = p.x, y = p.y, z = p.z;
			T xp = (m.m[0][0] * x + m.m[0][1] * y) + (m.m[0][2] * z + m.m[0][3]);
			T yp = (m.m[1][0] * x + m.m[1][1] * y) + (m.m[1][2] * z + m.m[1][3]);
			T zp = (m.m[2][0] * x + m.m[2][1] * y) + (m.m[2][2] * z + m.m[2][3]);
			T wp = (m.m[3][0] * x + m.m[3][1] * y) + (m.m[3][2] * z + m.m[3][3]);

			T xAbsSum = (std::abs(m.m[0][0] * x) + std::abs(m.m[0][1] * y) +
				std::abs(m.m[0][2] * z) + std::abs(m.m[0][3]));
			T yAbsSum = (std::abs(m.m[1][0] * x) + std::abs(m.m[1][1] * y) +
				std::abs(m.m[1][2] * z) + std::abs(m.m[1][3]));
			T zAbsSum = (std::abs(m.m[2][0] * x) + std::abs(m.m[2][1] * y) +
				std::abs(m.m[2][2] * z) + std::abs(m.m[2][3]));
			pError = gamma(3) * Vector3<T>(xAbsSum, yAbsSum, zAbsSum);
			if (wp == 1)
				return Point3<T>(xp, yp, zp);
			else
				return Point3<T>(xp, yp, zp) / wp;
		}

		template <typename T>
		inline Point3<T> operator()(const Point3<T> &pt, const Vector3<T> &ptError, Vector3<T> &absError) const
		{
			T x = pt.x, y = pt.y, z = pt.z;
			T xp = (m.m[0][0] * x + m.m[0][1] * y) + (m.m[0][2] * z + m.m[0][3]);
			T yp = (m.m[1][0] * x + m.m[1][1] * y) + (m.m[1][2] * z + m.m[1][3]);
			T zp = (m.m[2][0] * x + m.m[2][1] * y) + (m.m[2][2] * z + m.m[2][3]);
			T wp = (m.m[3][0] * x + m.m[3][1] * y) + (m.m[3][2] * z + m.m[3][3]);
			absError.x =
				(gamma(3) + (T)1) *
				(std::abs(m.m[0][0]) * ptError.x + std::abs(m.m[0][1]) * ptError.y +
					std::abs(m.m[0][2]) * ptError.z) +
				gamma(3) * (std::abs(m.m[0][0] * x) + std::abs(m.m[0][1] * y) +
					std::abs(m.m[0][2] * z) + std::abs(m.m[0][3]));
			absError.y =
				(gamma(3) + (T)1) *
				(std::abs(m.m[1][0]) * ptError.x + std::abs(m.m[1][1]) * ptError.y +
					std::abs(m.m[1][2]) * ptError.z) +
				gamma(3) * (std::abs(m.m[1][0] * x) + std::abs(m.m[1][1] * y) +
					std::abs(m.m[1][2] * z) + std::abs(m.m[1][3]));
			absError.z =
				(gamma(3) + (T)1) *
				(std::abs(m.m[2][0]) * ptError.x + std::abs(m.m[2][1]) * ptError.y +
					std::abs(m.m[2][2]) * ptError.z) +
				gamma(3) * (std::abs(m.m[2][0] * x) + std::abs(m.m[2][1] * y) +
					std::abs(m.m[2][2] * z) + std::abs(m.m[2][3]));
			if (wp == 1.)
				return Point3<T>(xp, yp, zp);
			else
				return Point3<T>(xp, yp, zp) / wp;
		}

		template <typename T>
		inline Vector3<T> operator()(const Vector3<T> &v, Vector3<T> &absError) const
		{
			T x = v.x, y = v.y, z = v.z;
			absError.x =
				gamma(3) * (std::abs(m.m[0][0] * v.x) + std::abs(m.m[0][1] * v.y) +
					std::abs(m.m[0][2] * v.z));
			absError.y =
				gamma(3) * (std::abs(m.m[1][0] * v.x) + std::abs(m.m[1][1] * v.y) +
					std::abs(m.m[1][2] * v.z));
			absError.z =
				gamma(3) * (std::abs(m.m[2][0] * v.x) + std::abs(m.m[2][1] * v.y) +
					std::abs(m.m[2][2] * v.z));
			return Vector3<T>(m.m[0][0] * x + m.m[0][1] * y + m.m[0][2] * z,
				m.m[1][0] * x + m.m[1][1] * y + m.m[1][2] * z,
				m.m[2][0] * x + m.m[2][1] * y + m.m[2][2] * z);
		}

		inline Ray operator()(const Ray &r, Vec3f &oError, Vec3f &dError) const
		{
			Point3f o = (*this)(r.origin, oError);
			Vec3f d = operator()(r.dir, dError);
			Float tMax = r.tmax;
			Float lengthSquared = d.lengthSquared();
			if (lengthSquared > 0)
			{
				Float dt = dot(abs(d), oError) / lengthSquared;
				o += d * dt;
			}
			return (Ray(o, d, tMax, r.time, r.medium));
		}

		Bounds3f operator()(const Bounds3f &b) const
		{
			const Transform &M = *this;
			Bounds3f ret(M(Point3f(b.min.x, b.min.y, b.min.z)));
			ret = expand(ret, M(Point3f(b.max.x, b.min.y, b.min.z)));
			ret = expand(ret, M(Point3f(b.min.x, b.max.y, b.min.z)));
			ret = expand(ret, M(Point3f(b.min.x, b.min.y, b.max.z)));
			ret = expand(ret, M(Point3f(b.min.x, b.max.y, b.max.z)));
			ret = expand(ret, M(Point3f(b.max.x, b.max.y, b.min.z)));
			ret = expand(ret, M(Point3f(b.max.x, b.min.y, b.max.z)));
			ret = expand(ret, M(Point3f(b.max.x, b.max.y, b.max.z)));
			return ret;
		}

		SurfaceInteraction operator()(const SurfaceInteraction &si) const
		{
			SurfaceInteraction ret;

			ret.p = (*this)(si.p, si.pError, ret.pError);

			const Transform &t = *this;
			ret.n = normalize(t(si.n));
			ret.wo = normalize(t(si.wo));
			ret.time = si.time;
			ret.mediumInterface = si.mediumInterface;
			ret.uv = si.uv;
			ret.shape = si.shape;
			ret.dpdu = t(si.dpdu);
			ret.dpdv = t(si.dpdv);
			ret.dndu = t(si.dndu);
			ret.dndv = t(si.dndv);
			ret.shading.n = normalize(t(si.shading.n));
			ret.shading.dpdu = t(si.shading.dpdu);
			ret.shading.dpdv = t(si.shading.dpdv);
			ret.shading.dndu = t(si.shading.dndu);
			ret.shading.dndv = t(si.shading.dndv);
			ret.dudx = si.dudx;
			ret.dvdx = si.dvdx;
			ret.dudy = si.dudy;
			ret.dvdy = si.dvdy;
			ret.dpdx = t(si.dpdx);
			ret.dpdy = t(si.dpdy);
			ret.bsdf = si.bsdf;
			ret.primitive = si.primitive;
			ret.shading.n = faceForward(ret.shading.n, ret.n);
			ret.faceIndex = si.faceIndex;
			return ret;
		}

		inline Ray operator()(const Ray &r) const
		{
			Vec3f oError;
			Point3f o = (*this)(r.origin, oError);
			Vec3f d = (*this)(r.dir);

			Float lengthSquared = d.lengthSquared();
			Float tMax = r.tmax;
			if (lengthSquared > 0)
			{
				Float dt = dot(abs(d), oError) / lengthSquared;
				o += d * dt;
				tMax -= dt;
			}
			return Ray(o, d, tMax, r.time, r.medium);
		}

		Transform operator*(const Transform &t2) const {
			return Transform(Matrix4x4::mul(m, t2.m), Matrix4x4::mul(t2.mInv, mInv));
		}

	private:
		Matrix4x4 m;
		Matrix4x4 mInv;
	};

	inline Transform translate(const Vec3f &delta)
	{
		Matrix4x4 m(1, 0, 0, delta.x, 0, 1, 0, delta.y, 0, 0, 1, delta.z, 0, 0, 0, 1);
		Matrix4x4 minv(1, 0, 0, -delta.x, 0, 1, 0, -delta.y, 0, 0, 1, -delta.z, 0, 0, 0, 1);
		return Transform(m, minv);
	}

	inline Transform scale(Float x, Float y, Float z)
	{
		Matrix4x4 m(x, 0, 0, 0, 0, y, 0, 0, 0, 0, z, 0, 0, 0, 0, 1);
		Matrix4x4 minv(1 / x, 0, 0, 0, 0, 1 / y, 0, 0, 0, 0, 1 / z, 0, 0, 0, 0, 1);
		return Transform(m, minv);
	}

	inline Transform rotateX(Float theta)
	{
		Float sinTheta = std::sin(radians(theta));
		Float cosTheta = std::cos(radians(theta));
		Matrix4x4 m(1, 0, 0, 0, 0, cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0,
			0, 0, 0, 1);
		return Transform(m, m.transpose());
	}

	inline Transform rotateY(Float theta)
	{
		Float sinTheta = std::sin(radians(theta));
		Float cosTheta = std::cos(radians(theta));
		Matrix4x4 m(cosTheta, 0, sinTheta, 0, 0, 1, 0, 0, -sinTheta, 0, cosTheta, 0,
			0, 0, 0, 1);
		return Transform(m, m.transpose());
	}

	inline Transform rotateZ(Float theta)
	{
		Float sinTheta = std::sin(radians(theta));
		Float cosTheta = std::cos(radians(theta));
		Matrix4x4 m(cosTheta, -sinTheta, 0, 0, sinTheta, cosTheta, 0, 0, 0, 0, 1, 0,
			0, 0, 0, 1);
		return Transform(m, m.transpose());
	}

	inline Transform rotate(Float theta, const Vec3f &axis)
	{
		Vec3f a = normalize(axis);
		Float sinTheta = std::sin(radians(theta));
		Float cosTheta = std::cos(radians(theta));
		Matrix4x4 m;
		m.m[0][0] = a.x * a.x + (1 - a.x * a.x) * cosTheta;
		m.m[0][1] = a.x * a.y * (1 - cosTheta) - a.z * sinTheta;
		m.m[0][2] = a.x * a.z * (1 - cosTheta) + a.y * sinTheta;
		m.m[0][3] = 0;

		m.m[1][0] = a.x * a.y * (1 - cosTheta) + a.z * sinTheta;
		m.m[1][1] = a.y * a.y + (1 - a.y * a.y) * cosTheta;
		m.m[1][2] = a.y * a.z * (1 - cosTheta) - a.x * sinTheta;
		m.m[1][3] = 0;

		m.m[2][0] = a.x * a.z * (1 - cosTheta) - a.y * sinTheta;
		m.m[2][1] = a.y * a.z * (1 - cosTheta) + a.x * sinTheta;
		m.m[2][2] = a.z * a.z + (1 - a.z * a.z) * cosTheta;
		m.m[2][3] = 0;
		return Transform(m, m.transpose());
	}

	inline Transform lookAt(const Point3f &pos, const Point3f &look, const Vec3f &up)
	{
		Matrix4x4 cameraToWorld;
	
		cameraToWorld.m[0][3] = pos.x;
		cameraToWorld.m[1][3] = pos.y;
		cameraToWorld.m[2][3] = pos.z;
		cameraToWorld.m[3][3] = 1;

		Vec3f dir = normalize(look - pos);
		if (cross(normalize(up), dir).length() == 0)
		{
			return Transform();
		}

		Vec3f right = normalize(cross(normalize(up), dir));
		Vec3f newUp = cross(dir, right);
		cameraToWorld.m[0][0] = right.x;
		cameraToWorld.m[1][0] = right.y;
		cameraToWorld.m[2][0] = right.z;
		cameraToWorld.m[3][0] = 0.;
		cameraToWorld.m[0][1] = newUp.x;
		cameraToWorld.m[1][1] = newUp.y;
		cameraToWorld.m[2][1] = newUp.z;
		cameraToWorld.m[3][1] = 0.;
		cameraToWorld.m[0][2] = dir.x;
		cameraToWorld.m[1][2] = dir.y;
		cameraToWorld.m[2][2] = dir.z;
		cameraToWorld.m[3][2] = 0.;
		return Transform(cameraToWorld.inverse(), cameraToWorld);
	}

	inline Transform orthographic(Float zNear, Float zFar)
	{
		return scale(1, 1, 1 / (zFar - zNear)) * translate(Vec3f(0, 0, -zNear));
	}

	inline Transform perspective(Float fov, Float n, Float f)
	{
		Matrix4x4 persp(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, f / (f - n), -f * n / (f - n),
			0, 0, 1, 0);

		Float invTanAng = 1 / std::tan(radians(fov) / 2);
		return scale(invTanAng, invTanAng, 1) * Transform(persp);
	}

	inline Transform perspective(Float vFov, Float hFov, Float n, Float f)
	{
		Matrix4x4 persp(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, f / (f - n), -f * n / (f - n),
			0, 0, 1, 0);

		Float vInvTanAng = 1 / std::tan(radians(vFov) / 2);
		Float hInvTanAng = 1 / std::tan(radians(hFov) / 2);
		return scale(hInvTanAng, vInvTanAng, 1) * Transform(persp);
	}

	inline std::ostream &operator<<(std::ostream &os, const Transform &t)
	{
		os << "{" << t.getMatrix().m[0][0] << ", " << t.getMatrix().m[0][1] << ", " << t.getMatrix().m[0][2] << ", " << t.getMatrix().m[0][3] << "}"
			<< "{" << t.getMatrix().m[1][0] << ", " << t.getMatrix().m[1][1] << ", " << t.getMatrix().m[1][2] << ", " << t.getMatrix().m[1][3] << "}"
			<< "{" << t.getMatrix().m[2][0] << ", " << t.getMatrix().m[2][1] << ", " << t.getMatrix().m[2][2] << ", " << t.getMatrix().m[2][3] << "}"
			<< "{" << t.getMatrix().m[3][0] << ", " << t.getMatrix().m[3][1] << ", " << t.getMatrix().m[3][2] << ", " << t.getMatrix().m[3][3] << "}";
		return (os);
	}
}