#include "atlas/core/FilmIterator.h"

using namespace atlas;

FilmIterator::FilmIterator(Film::Pixel *pixels, uint32_t size)
	: pixels(pixels)
	, size(size)
{}

void FilmIterator::start(uint32_t newSampleCount)
{
	if (itCount != 0)
		save();
	sampleCount = newSampleCount;
	clear(sampleCount);
	itCount++;
}

void FilmIterator::clear(uint32_t sampleCount)
{
	for (uint32_t i = 0; i < size; i++)
	{
		pixels[i].color = BLACK;
		pixels[i].filterWeightSum = sampleCount;
	}
}

void FilmIterator::save()
{
	uint32_t flags = CREATE_ALWAYS;
	std::string filename = "iteration-" + std::to_string(itCount) + ".tmp";
	const size_t byteSize = sizeof(Film::Pixel) * size;
	void *file = CreateFileA(filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	CHECK_WIN_CALL(file != INVALID_HANDLE_VALUE);
	void *mapping = CreateFileMappingA(file, nullptr, PAGE_READWRITE, 0,  byteSize, nullptr);
	CHECK_WIN_CALL(mapping != INVALID_HANDLE_VALUE);
	Film::Pixel *buffer = (Film::Pixel *)MapViewOfFile(mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, byteSize);
	CHECK_WIN_CALL(buffer != nullptr);

	for (uint32_t i = 0; i < size; i++)
	{
		buffer[i] = pixels[i];
	}

	FlushViewOfFile(buffer, byteSize);
	UnmapViewOfFile(buffer);
	CloseHandle(mapping);
	CloseHandle(file);
}

void FilmIterator::accumulate()
{
	for (uint32_t i = 1; i < itCount; i++)
	{
		uint32_t flags = OPEN_EXISTING;
		std::string filename = "iteration-" + std::to_string(i) + ".tmp";
		const size_t byteSize = sizeof(Film::Pixel) * size;
		void *file = CreateFileA(filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
		CHECK_WIN_CALL(file != INVALID_HANDLE_VALUE);
		void *mapping = CreateFileMappingA(file, nullptr, PAGE_READWRITE, 0, byteSize, nullptr);
		CHECK_WIN_CALL(mapping != INVALID_HANDLE_VALUE);
		Film::Pixel *buffer = (Film::Pixel *)MapViewOfFile(mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, byteSize);
		CHECK_WIN_CALL(buffer != nullptr);

		for (uint32_t i = 0; i < size; i++)
		{
			pixels[i].color += buffer[i].color;
			pixels[i].filterWeightSum += buffer[i].filterWeightSum;
		}

		UnmapViewOfFile(buffer);
		CloseHandle(mapping);
		CloseHandle(file);
	}
}