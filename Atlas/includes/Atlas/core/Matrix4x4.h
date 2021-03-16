#pragma once

#include "atlas/Atlas.h"
#include "atlas/AtlasLibHeader.h"

namespace atlas
{
	struct Matrix4x4
	{
		Matrix4x4() = default;
		ATLAS Matrix4x4(Float d[4][4]);
		ATLAS Matrix4x4(Float m00, Float m01, Float m02, Float m03,
						Float m10, Float m11, Float m12, Float m13,
						Float m20, Float m21, Float m22, Float m23,
						Float m30, Float m31, Float m32, Float m33);
		ATLAS Matrix4x4(const Matrix4x4 &matrix);

		ATLAS Matrix4x4 transpose() const;
		ATLAS Matrix4x4 inverse() const;

		ATLAS bool operator==(const Matrix4x4 &t) const;
		ATLAS bool operator!=(const Matrix4x4 &t) const;
		ATLAS Matrix4x4 operator*(const Matrix4x4 &m2);

		static Matrix4x4 mul(const Matrix4x4 &m1, const Matrix4x4 &m2)
		{
			Matrix4x4 r;
			for (int i = 0; i < 4; ++i)
				for (int j = 0; j < 4; ++j)
					r.m[i][j] = m1.m[i][0] * m2.m[0][j] + m1.m[i][1] * m2.m[1][j] +
					m1.m[i][2] * m2.m[2][j] + m1.m[i][3] * m2.m[3][j];
			return r;
		}

		Float m[4][4];
	};
}