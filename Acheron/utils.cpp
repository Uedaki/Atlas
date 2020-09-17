#include "pch.h"
#include "utils.h"

uint64_t posToZOrderIndex(uint32_t x, uint32_t y)
{
	//uint64_t index = 0;
	//for (uint8_t i = 0; i < sizeof(uint32_t) * 8; i++)
	//{
	//	index |= (((static_cast<uint64_t>(x) >> i) & 1) << i * 2);
	//	index |= (((static_cast<uint64_t>(y) >> i) & 1) << i * 2 + 1);
	//}
	return (_pdep_u32(x, 0x55555555) | _pdep_u32(y, 0xaaaaaaaa));
}

uint64_t zOrderIndexToPos(uint64_t index, uint32_t &x, uint32_t &y)
{
	x = _pext_u64(index, 0x5555555555555555);;
	y = _pext_u64(index, 0xaaaaaaaaaaaaaaaa);

	//for (uint64_t i = 0; i < sizeof(uint64_t) * 8; i += 2)
	//{
	//	x |= (((static_cast<uint64_t>(index) >> i) & 1) << i / 2);
	//}

	//for (uint64_t i = 1; i < sizeof(uint64_t) * 8; i += 2)
	//{
	//	y |= (((static_cast<uint64_t>(index) >> i) & 1) << i / 2);
	//}
	return (index);
}

uint32_t toRgb9e5(const glm::vec3 &weight)
{
	uint32_t cw = 0;

	const float N = 9; // N is the number of mantissa bits per component
	const float Emax = 31; // Emax is the maximum allowed biased exponent value
	const float B = 15; // B is the exponent bias
	const float sharedexp_max = (pow(2, N) - 1) / pow(2, N) * pow(2, Emax - B);

	const float red_c = std::max(0.f, std::min(sharedexp_max, weight.r));
	const float green_c = std::max(0.f, std::min(sharedexp_max, weight.g));
	const float blue_c = std::max(0.f, std::min(sharedexp_max, weight.b));

	const float max_c = std::max(std::max(red_c, green_c), blue_c);

	float exp_shared_p = std::max(-B - 1, floor(log2(max_c))) + 1 + B;

	const float max_s = floor(max_c / pow(2, (exp_shared_p - B - N)) + 0.5);
	const float exp_shared = max_s == pow(2, N) ? exp_shared_p + 1 : exp_shared_p;

	const float red_s = floor(red_c / pow(2, (exp_shared - B - N)) + 0.5);
	const float green_s = floor(green_c / pow(2, (exp_shared - B - N)) + 0.5);
	const float blue_s = floor(blue_c / pow(2, (exp_shared - B - N)) + 0.5);

	copyUintToBit(red_s, cw, 9, 0);
	copyUintToBit(green_s, cw, 9, 9);
	copyUintToBit(blue_s, cw, 9, 18);
	copyUintToBit(exp_shared, cw, 5, 27);

	return (cw);
}

glm::vec3 toColor(uint32_t weight)
{
	glm::vec3 color;

	const float N = 9; // N is the number of mantissa bits per component
	const float B = 15; // B is the exponent bias

	const float red_s = copyBitToUint(weight, 9, 0);
	const float green_s = copyBitToUint(weight, 9, 9);
	const float blue_s = copyBitToUint(weight, 9, 18);
	const float exp_shared = copyBitToUint(weight, 5, 27);

	color.r = red_s * pow(2, exp_shared - B - N);
	color.g = green_s * pow(2, exp_shared - B - N);
	color.b = blue_s * pow(2, exp_shared - B - N);

	return (color);
}

void copyUintToBit(uint32_t src, uint32_t &dst, uint32_t size, uint32_t offset)
{
	for (uint32_t i = 0; i < size; i++)
	{
		dst |= (((src >> i) & 1) << (offset + i));
	}
}

uint32_t copyBitToUint(uint32_t src, uint32_t size, uint32_t offset)
{
	uint32_t dst = 0;
	for (uint32_t i = 0; i < size; i++)
	{
		dst |= (((src >> (offset + i)) & 1) << i);
	}
	return (dst);
}

glm::vec2 OctWrap(glm::vec2 v)
{
	return (glm::vec2((1.0 - abs(v.y)) * (v.x >= 0.0 ? 1.0 : -1.0),
		(1.0 - abs(v.x)) * (v.y >= 0.0 ? 1.0 : -1.0)));
}

glm::vec2 octEncode(glm::vec3 n)
{
	//float l1norm = abs(n.x) + abs(n.y) + abs(n.z);
	//float invNorm = 1.f / l1norm;
	//glm::vec2 result(n.x * invNorm, n.y * invNorm);
	//if (n.z < 0.f)
	//{
	//	float xSign = result.x >= 0 ? 1.f : -1.f;
	//	float ySign = result.y >= 0 ? 1.f : -1.f;

	//	result.x = (1 - abs(result.y)) * xSign;
	//	result.y = (1 - abs(result.x)) * ySign;
	//}
	//return (result);

	glm::vec3 r = n / (abs(n.x) + abs(n.y) + abs(n.z));
	glm::vec2 c = r.z >= 0.0 ? glm::vec2(r.x, r.y) : OctWrap(glm::vec2(r.x, r.y));
	c.x = c.x * 0.5 + 0.5;
	c.y = c.y * 0.5 + 0.5;
	return (c);
}

glm::vec3 octDecode(glm::vec2 f)
{
	/*glm::vec3 v(f.x, f.y, 1 - abs(f.x) - abs(f.y));
	if (v.z < 0.f)
	{
		float xSign = v.x >= 0 ? 1.f : -1.f;
		float ySign = v.y >= 0 ? 1.f : -1.f;

		v.x = (1 - abs(v.y)) * xSign;
		v.y = (1 - abs(v.x)) * ySign;
	}
	return (glm::normalize(v));	*/

	f = f * 2.0f - 1.0f;

	// https://twitter.com/Stubbesaurus/status/937994790553227264
	glm::vec3 n(f.x, f.y, 1.0 - abs(f.x) - abs(f.y));
	float t = std::max(-n.z, 0.f);
	n.x += n.x >= 0.0 ? -t : t;
	n.y += n.y >= 0.0 ? -t : t;
	return glm::normalize(n);
}

float maxValue(const glm::vec3 &v)
{
	return (v.x > v.y ? (v.x > v.z ? v.x : v.z) : (v.y > v.z ? v.y : v.z));
}

float maxIdx(const glm::vec3 &v)
{
	return (v.x > v.y ? (v.x > v.z ? 0 : 1) : (v.y > v.z ? 1 : 2));
}