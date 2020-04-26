#pragma once

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#pragma region SIMD

#define VM_INLINE __forceinline

struct float4
{
	VM_INLINE float4() = default;
	VM_INLINE explicit float4(const float *p) { m = _mm_loadu_ps(p); }
	VM_INLINE explicit float4(float x, float y, float z, float w) { m = _mm_set_ps(w, z, y, x); }
	VM_INLINE explicit float4(float v) { m = _mm_set_ps1(v); }
	VM_INLINE explicit float4(__m128 v) { m = v; }

	__m128 m;
};

VM_INLINE float4 operator+(float4 a, float4 b) { return (float4(_mm_add_ps(a.m, b.m))); }
VM_INLINE float4 operator-(float4 a, float4 b) { return (float4(_mm_sub_ps(a.m, b.m))); }
VM_INLINE float4 operator*(float4 a, float4 b) { return (float4(_mm_mul_ps(a.m, b.m))); }
VM_INLINE float4 operator/(float4 a, float4 b) { return (float4(_mm_div_ps(a.m, b.m))); }
VM_INLINE float4 operator==(float4 a, float4 b) { return (float4(_mm_cmpeq_ps(a.m, b.m))); }
VM_INLINE float4 operator!=(float4 a, float4 b) { return (float4(_mm_cmpneq_ps(a.m, b.m))); }
VM_INLINE float4 operator<(float4 a, float4 b) { return (float4(_mm_cmplt_ps(a.m, b.m))); }
VM_INLINE float4 operator>(float4 a, float4 b) { return (float4(_mm_cmpgt_ps(a.m, b.m))); }
VM_INLINE float4 operator<=(float4 a, float4 b) { return (float4(_mm_cmple_ps(a.m, b.m))); }
VM_INLINE float4 operator>=(float4 a, float4 b) { return (float4(_mm_cmpge_ps(a.m, b.m))); }
VM_INLINE float4 operator&(float4 a, float4 b) { return (float4(_mm_and_ps(a.m, b.m))); }
VM_INLINE float4 operator|(float4 a, float4 b) { return (float4(_mm_or_ps(a.m, b.m))); }
VM_INLINE float4 operator-(float4 a) { return (float4(_mm_xor_ps(a.m, _mm_set1_ps(-0.0f)))); }
VM_INLINE float4 min(float4 a, float4 b) { return (float4(_mm_min_ps(a.m, b.m))); }
VM_INLINE float4 max(float4 a, float4 b) { return (float4(_mm_max_ps(a.m, b.m))); }

#define SHUFFLE4(V, X, Y, Z, W) float4(_mm_shuffle_ps((V).m, (V).m, _MM_SHUFFLE(W, Z, Y, X)))

typedef float4 bool4;

VM_INLINE unsigned mask(float4 v) { return _mm_movemask_ps(v.m) & 15; }
VM_INLINE bool any(bool4 v) { return (mask(v) != 0); }
VM_INLINE bool all(bool4 v) { return (mask(v) == 15); }

VM_INLINE float4 select(float4 a, float4 b, bool4 cond)
{
	return (float4(_mm_blendv_ps(a.m, b.m, cond.m)));
}

VM_INLINE __m128i select(__m128i a, __m128i b, bool4 cond)
{
	return (_mm_blendv_epi8(a, b, _mm_castps_si128(cond.m)));
}

VM_INLINE float4 sqrtf(float4 v) { return (float4(_mm_sqrt_ps(v.m))); }

#pragma endregion

struct SimdRay
{
	float4 origX;
	float4 origY;
	float4 origZ;

	float4 dirX;
	float4 dirY;
	float4 dirZ;

	float4 invDirX;
	float4 invDirY;
	float4 invDirZ;

	float4 signX;
	float4 signY;
	float4 signZ;

	SimdRay() = default;
	SimdRay(float4 posX, float4 posY, float4 posZ, float4 dirX, float4 dirY, float4 dirZ)
		: origX(posX), origY(posY), origZ(posZ)
		, dirX(dirX), dirY(dirY), dirZ(dirZ)
	{
		float4 unit(1.f);
		float4 zero(0.f);

		invDirX = unit / dirX;
		invDirY = unit / dirY;
		invDirZ = unit / dirZ;

		signX = invDirX < zero;
		signY = invDirY < zero;
		signZ = invDirZ < zero;
	}
};

struct SimdTexel
{
	float4 r;
	float4 g;
	float4 b;
};

struct SimdHitRecord
{
	float4 t;
	
	float4 pX;
	float4 pY;
	float4 pZ;

	float4 normalX;
	float4 normalY;
	float4 normalZ;

	SimdTexel texel;
};