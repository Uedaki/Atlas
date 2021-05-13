#include "ExtractBatch.h"

bool atlas::task::ExtractBatch::preExecute()
{
	filename = data.batchManager->popBatchName();
	data.dst->resize(0);
	size = 0;

	if (filename.empty())
	{
		Bin *uncompletedBin = nullptr;
		if ((uncompletedBin = data.batchManager->getUncompledBatch()))
		{
			filename = uncompletedBin->filename;
			size = uncompletedBin->pos;
			handle = uncompletedBin->currentFile;

			uncompletedBin->filename = data.batchManager->getNewBatchName();
			Bin::open(*uncompletedBin, data.batchManager->getBinSize());
			Bin::unmap(uncompletedBin->currentFile, data.batchManager->getBinSize());
		}
		else
			return (false);
	}
	else
		size = data.batchManager->getBinSize();

	const DWORD byteSize = sizeof(uint32_t) + size * sizeof(CompactRay);

	uint32_t flags = OPEN_EXISTING;

	handle.file = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY, nullptr);
	CHECK_WIN_CALL(handle.file != INVALID_HANDLE_VALUE);

	handle.mapping = CreateFileMappingA(handle.file, nullptr, PAGE_READONLY, 0, byteSize, nullptr);
	CHECK_WIN_CALL(handle.mapping);

	handle.buffer = (CompactRay *)MapViewOfFile(handle.mapping, FILE_MAP_READ, 0, 0, byteSize);
	CHECK_WIN_CALL(handle.buffer);

	data.dst->resize(size);
	return (true);
}

void atlas::task::ExtractBatch::execute()
{
	if (size == 0)
		return;

	while (true)
	{
		uint32_t offset = index.fetch_add(maxRayPerPass);
		if (offset >= size)
			break;

		uint32_t end = std::min(offset + maxRayPerPass, size);
		for (uint32_t i = offset; i < end; i++)
		{
			data.dst->origins[i] = handle.buffer[i].origin;
			data.dst->directions[i] = octDecode(handle.buffer[i].direction);
			data.dst->colors[i] = toColor(handle.buffer[i].weight);
			data.dst->pixelIDs[i] = handle.buffer[i].pixelID;
			data.dst->sampleIDs[i] = handle.buffer[i].sampleID;
			data.dst->depths[i] = handle.buffer[i].depth;
			data.dst->tNears[i] = handle.buffer[i].tNear;
		}
	}
}

void atlas::task::ExtractBatch::postExecute()
{
	if (size == 0)
		return;
	Bin::unmap(handle, data.batchManager->getBinSize());
}