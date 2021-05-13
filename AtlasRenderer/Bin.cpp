#include "Bin.h"

// Need to be above of the other include
#include <windows.h>

#include <FileApi.h>
#include <MemoryApi.h>
#include <string>

using namespace atlas;

void Bin::open(Bin &bin, uint32_t maxSize)
{
	DCHECK(pow(2, log2(maxSize)) == maxSize);

	if (bin.prevFile.file)
		Bin::unmap(bin.prevFile, maxSize);
	bin.prevFile = bin.currentFile;

	uint32_t flags = CREATE_ALWAYS;
	const size_t size = sizeof(uint32_t) + maxSize * sizeof(CompactRay);
	bin.currentFile.file = CreateFileA(bin.filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	CHECK_WIN_CALL(bin.currentFile.file != INVALID_HANDLE_VALUE);
	bin.currentFile.mapping = CreateFileMappingA(bin.currentFile.file, nullptr, PAGE_READWRITE, 0, (DWORD)size, nullptr);
	CHECK_WIN_CALL(bin.currentFile.mapping != INVALID_HANDLE_VALUE);
	bin.currentFile.buffer = (CompactRay *)MapViewOfFile(bin.currentFile.mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
	CHECK_WIN_CALL(bin.currentFile.buffer != nullptr);

	bin.pos = 0;
}

void Bin::map(Bin &bin, uint32_t maxSize)
{
	DCHECK(pow(2, log2(maxSize)) == maxSize);

	if (bin.prevFile.file)
		Bin::unmap(bin.prevFile, maxSize);
	bin.prevFile = bin.currentFile;

	uint32_t flags = OPEN_EXISTING;
	const size_t size = sizeof(uint32_t) + maxSize * sizeof(CompactRay);
	bin.currentFile.file = CreateFileA(bin.filename.c_str(), GENERIC_WRITE | GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_TEMPORARY, nullptr);
	CHECK_WIN_CALL(bin.currentFile.file != INVALID_HANDLE_VALUE);
	bin.currentFile.mapping = CreateFileMappingA(bin.currentFile.file, nullptr, PAGE_READWRITE, 0, (DWORD)size, nullptr);
	CHECK_WIN_CALL(bin.currentFile.mapping != INVALID_HANDLE_VALUE);
	bin.currentFile.buffer = (CompactRay *)MapViewOfFile(bin.currentFile.mapping, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, size);
	CHECK_WIN_CALL(bin.currentFile.buffer != nullptr);
}

void Bin::unmap(Bin::FileHandles &handles, uint32_t maxSize)
{
	const size_t size = maxSize * sizeof(CompactRay);
	FlushViewOfFile(handles.buffer, size);
	UnmapViewOfFile(handles.buffer);
	CloseHandle(handles.mapping);
	CloseHandle(handles.file);
	handles = { 0 };
}

void Bin::reset(Bin &bin)
{

}