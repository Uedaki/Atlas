#pragma once

#ifdef _USE_SIMD

#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>

#define SIMD_INLINE __forceinline

struct S4Float
{
	SIMD_INLINE S4Float() = default;
	SIMD_INLINE explicit S4Float(const float *p) {
		m = _mm_loadu_ps(p);
	}
	SIMD_INLINE explicit S4Float(float x, float y, float z, float w) {
		m = _mm_set_ps(w, z, y, x);
	}
	SIMD_INLINE explicit S4Float(float v) {
		m = _mm_set_ps1(v);
	}
	SIMD_INLINE explicit S4Float(__m128 v) {
		m = v;
	}

	__m128 m;
};

SIMD_INLINE S4Float operator+(S4Float a, S4Float b)
{
	return (S4Float(_mm_add_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator-(S4Float a, S4Float b)
{
	return (S4Float(_mm_sub_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator*(S4Float a, S4Float b)
{
	return (S4Float(_mm_mul_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator/(S4Float a, S4Float b)
{
	return (S4Float(_mm_div_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator==(S4Float a, S4Float b)
{
	return (S4Float(_mm_cmpeq_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator!=(S4Float a, S4Float b)
{
	return (S4Float(_mm_cmpneq_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator<(S4Float a, S4Float b)
{
	return (S4Float(_mm_cmplt_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator>(S4Float a, S4Float b)
{
	return (S4Float(_mm_cmpgt_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator<=(S4Float a, S4Float b)
{
	return (S4Float(_mm_cmple_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator>=(S4Float a, S4Float b)
{
	return (S4Float(_mm_cmpge_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator&(S4Float a, S4Float b)
{
	return (S4Float(_mm_and_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator|(S4Float a, S4Float b)
{
	return (S4Float(_mm_or_ps(a.m, b.m)));
}

SIMD_INLINE S4Float operator-(S4Float a)
{
	return (S4Float(_mm_xor_ps(a.m, _mm_set1_ps(-0.0f))));
}

SIMD_INLINE S4Float min(S4Float a, S4Float b)
{
	return (S4Float(_mm_min_ps(a.m, b.m)));
}

SIMD_INLINE S4Float max(S4Float a, S4Float b)
{
	return (S4Float(_mm_max_ps(a.m, b.m)));
}

#define SHUFFLE4(V, X, Y, Z, W) S4Float(_mm_shuffle_ps((V).m, (V).m, _MM_SHUFFLE(W, Z, Y, X)))

typedef S4Float S4Bool;

SIMD_INLINE unsigned mask(S4Float v)
{
	return _mm_movemask_ps(v.m) & 15;
}

SIMD_INLINE bool any(S4Bool v)
{
	return (mask(v) != 0);
}

SIMD_INLINE bool all(S4Bool v)
{
	return (mask(v) == 15);
}

SIMD_INLINE S4Float select(S4Float a, S4Float b, S4Bool cond)
{
	return (S4Float(_mm_blendv_ps(a.m, b.m, cond.m)));
}

SIMD_INLINE __m128i select(__m128i a, __m128i b, S4Bool cond)
{
	return (_mm_blendv_epi8(a, b, _mm_castps_si128(cond.m)));
}

SIMD_INLINE S4Float sqrtf(S4Float v)
{
	return (S4Float(_mm_sqrt_ps(v.m)));
}

SIMD_INLINE S4Float atan()
{

}

#endif