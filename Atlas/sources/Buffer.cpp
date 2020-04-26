#include "pch.h"
#include "Buffer.h"

atlas::Buffer::Buffer(uint32_t size)
	: image(size), albedo(size), normal(size)
{}

void atlas::Buffer::resize(uint32_t size)
{
	image.resize(size);
	albedo.resize(size);
	normal.resize(size);
}