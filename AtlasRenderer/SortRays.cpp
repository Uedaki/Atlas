#include "SortRays.h"

bool atlas::task::SortRays::preExecute()
{
	if (data.batch->size() < 64)
		return (false);

	pushIndex = 1;
	nextIndex = 1;
	pullIndex = 0;

	sortingPacks.resize(data.batch->size());

	sortingPacks[0].start = 0;
	sortingPacks[0].end = data.batch->size() - 1;

	sortingPacks[0].target = data.batch->size() > 64 ? 4096 : 64;
	sortingPacks[0].order = data.batch->size() > 64 ? reinterpret_cast<Block<Vec3f> *>(&data.batch->origins) : &data.batch->directions;

	isRunning = true;
	return (true);
}

void atlas::task::SortRays::execute()
{
	CHECK(sortingPacks.size() != 0);

	SortingPack pack;
	if (isRunning && !getNextPack(pack))
	{ // lock scope
		std::unique_lock<std::mutex> lock(guard);
		sleepCtrl.wait(lock);

		if (!isRunning || !getNextPack(pack))
			return;
	}
	else if (!isRunning)
		return;

	while (true)
	{
		uint32_t size = pack.end - pack.start + 1;
		uint32_t med = size >> 1;
		std::vector<uint32_t> ref(size);
		for (uint32_t i = 0; i < size; i++)
			ref[i] = pack.start + i;

		uint8_t axis = pickLargestAxis(*pack.order, pack.start, pack.end);

		std::nth_element(ref.begin(), ref.begin() + med, ref.end(), [&pack, axis](const uint32_t idx1, const uint32_t idx2)
			{
				return ((*pack.order)[idx1][axis] < (*pack.order)[idx2][axis]);
			});

		for (uint32_t i = 0; i < size; i++)
		{
			if (i < ref[i])
			{
				data.batch->swap(pack.start + i, ref[i]);
			}
			else if (ref[i] < i)
			{
				uint32_t ib = i;
				while (ref[ib] < i)
				{
					ib = ref[ib];
				}
				data.batch->swap(pack.start + i, ref[ref[ib]]);
			}
		}

		if (med > 64)
		{
			SortingPack newPack;
			newPack.order = med >= 4096 ? reinterpret_cast<Block<Vec3f> *>(&data.batch->origins) : &data.batch->directions;
			newPack.start = pack.start + med + 1;
			newPack.end = pack.end;
			pushPack(newPack);

			pack.order = med >= 4096 ? reinterpret_cast<Block<Vec3f> *>(&data.batch->origins) : &data.batch->directions;
			pack.end = pack.start + med;
		}
		else if (!getNextPack(pack))
			break;
	}

	isRunning = false;
	sleepCtrl.notify_all();
}

void atlas::task::SortRays::postExecute()
{
	sortingPacks.clear();
}

uint8_t atlas::task::SortRays::pickLargestAxis(const Block<Vec3f> &ar, uint32_t firstIndex, uint32_t lastIndex) const
{
	Vec2f rangeX(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	Vec2f rangeY(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	Vec2f rangeZ(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
	for (uint32_t i = firstIndex; i <= lastIndex; i++)
	{
		rangeX[0] = std::min(rangeX[0], ar[i].x);
		rangeX[1] = std::max(rangeX[1], ar[i].x);

		rangeY[0] = std::min(rangeY[0], ar[i].y);
		rangeY[1] = std::max(rangeY[1], ar[i].y);

		rangeZ[0] = std::min(rangeZ[0], ar[i].z);
		rangeZ[1] = std::max(rangeZ[1], ar[i].z);
	}

	const float lengthX = rangeX[1] - rangeX[0];
	const float lengthY = rangeY[1] - rangeY[0];
	const float lengthZ = rangeZ[1] - rangeZ[0];

	return (lengthX < lengthY ? (lengthY < lengthZ ? 2 : 1) : (lengthX < lengthZ ? 2 : 0));
}