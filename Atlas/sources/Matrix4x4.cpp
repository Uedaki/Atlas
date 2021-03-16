#include "atlas/core/Matrix4x4.h"

#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

using namespace atlas;

Matrix4x4::Matrix4x4(Float d[4][4])
{
	memcpy(m, d, 16 * sizeof(Float));
}

Matrix4x4::Matrix4x4(Float m00, Float m01, Float m02, Float m03,
	Float m10, Float m11, Float m12, Float m13,
	Float m20, Float m21, Float m22, Float m23,
	Float m30, Float m31, Float m32, Float m33)
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;
	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;
	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;
	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

Matrix4x4::Matrix4x4(const Matrix4x4 &matrix)
{
	memcpy(m, matrix.m, 16 * sizeof(Float));
}

bool Matrix4x4::operator==(const Matrix4x4 &t) const
{
	return (m[0][0] == t.m[0][0] && m[0][1] == t.m[0][1] && m[0][2] == t.m[0][2] && m[0][3] == t.m[0][3]
		&& m[1][0] == t.m[1][0] && m[1][1] == t.m[1][1] && m[1][2] == t.m[1][2] && m[1][3] == t.m[1][3]
		&& m[2][0] == t.m[2][0] && m[2][1] == t.m[2][1] && m[2][2] == t.m[2][2] && m[2][3] == t.m[2][3]
		&& m[3][0] == t.m[3][0] && m[3][1] == t.m[3][1] && m[3][2] == t.m[3][2] && m[3][3] == t.m[3][3]);
}

bool Matrix4x4::operator!=(const Matrix4x4 &t) const
{
	return (m[0][0] != t.m[0][0] && m[0][1] != t.m[0][1] && m[0][2] != t.m[0][2] && m[0][3] != t.m[0][3]
		&& m[1][0] != t.m[1][0] && m[1][1] != t.m[1][1] && m[1][2] != t.m[1][2] && m[1][3] != t.m[1][3]
		&& m[2][0] != t.m[2][0] && m[2][1] != t.m[2][1] && m[2][2] != t.m[2][2] && m[2][3] != t.m[2][3]
		&& m[3][0] != t.m[3][0] && m[3][1] != t.m[3][1] && m[3][2] != t.m[3][2] && m[3][3] != t.m[3][3]);
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4 &m2)
{
	Matrix4x4 r;
	for (int8_t i = 0; i < 4; i++)
	{
		for (int8_t j = 0; j < 4; j++)
		{
			r.m[i][j] = m[i][0] * m2.m[0][j]
				+ m[i][1] * m2.m[1][j]
				+ m[i][2] * m2.m[2][j]
				+ m[i][3] * m2.m[3][j];
		}
	}
	return (r);
}

Matrix4x4 Matrix4x4::transpose() const
{
	return (Matrix4x4(m[0][0], m[1][0], m[2][0], m[3][0],
		m[0][1], m[1][1], m[2][1], m[3][1],
		m[0][2], m[1][2], m[2][2], m[3][2],
		m[0][3], m[1][3], m[2][3], m[3][3]));
}

// pbrt v3 inverse function (ref: https://github.com/mmp/pbrt-v3/blob/master/src/core/transform.cpp)
Matrix4x4 Matrix4x4::inverse() const
{
    int indxc[4], indxr[4];
    int ipiv[4] = { 0, 0, 0, 0 };
    Float minv[4][4];
    memcpy(minv, m, 4 * 4 * sizeof(Float));
    for (int i = 0; i < 4; i++) {
        int irow = 0, icol = 0;
        Float big = 0.f;
        // Choose pivot
        for (int j = 0; j < 4; j++) {
            if (ipiv[j] != 1) {
                for (int k = 0; k < 4; k++) {
                    if (ipiv[k] == 0) {
                        if (std::abs(minv[j][k]) >= big) {
                            big = Float(std::abs(minv[j][k]));
                            irow = j;
                            icol = k;
                        }
                    }
                    /*else if (ipiv[k] > 1)
                        Error("Singular matrix in MatrixInvert");*/
                }
            }
        }
        ++ipiv[icol];
        // Swap rows _irow_ and _icol_ for pivot
        if (irow != icol) {
            for (int k = 0; k < 4; ++k) std::swap(minv[irow][k], minv[icol][k]);
        }
        indxr[i] = irow;
        indxc[i] = icol;
        //if (minv[icol][icol] == 0.f) Error("Singular matrix in MatrixInvert");

        // Set $m[icol][icol]$ to one by scaling row _icol_ appropriately
        Float pivinv = 1. / minv[icol][icol];
        minv[icol][icol] = 1.;
        for (int j = 0; j < 4; j++) minv[icol][j] *= pivinv;

        // Subtract this row from others to zero out their columns
        for (int j = 0; j < 4; j++) {
            if (j != icol) {
                Float save = minv[j][icol];
                minv[j][icol] = 0;
                for (int k = 0; k < 4; k++) minv[j][k] -= minv[icol][k] * save;
            }
        }
    }
    // Swap columns to reflect permutation
    for (int j = 3; j >= 0; j--) {
        if (indxr[j] != indxc[j]) {
            for (int k = 0; k < 4; k++)
                std::swap(minv[k][indxr[j]], minv[k][indxc[j]]);
        }
    }
    return Matrix4x4(minv);
}