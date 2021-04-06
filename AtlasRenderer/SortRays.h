#pragma once

#include "Atlas/Atlas.h"
#include "Atlas/core/Points.h"
#include "Atlas/core/Vectors.h"
#include "Batch.h"
#include "ThreadPool.h"

namespace atlas
{
	namespace task
	{
		class SortRays : public ThreadedTask
		{
			struct Pack
			{
				std::vector<Vec3f> *order;
				uint32_t firstIndex;
				uint32_t size;
				uint32_t target;
			};

			struct SortingPack
			{
				int32_t start;
				int32_t end;

				int32_t target;
				std::vector<Vec3f> *order;
			};
			
		public:
			struct Data
			{
				Batch *batch;
			};

			SortRays(Data &data)
				: data(data)
			{}

			bool preExecute() override
			{
				pushIndex = 1;
				pullIndex = 0;

				sortingPacks.resize(data.batch->size());

				sortingPacks[0].start = 0;
				sortingPacks[0].end = data.batch->size() - 1;

				sortingPacks[0].target = data.batch->size() > 64 ? 4096 : 64;
				sortingPacks[0].order = data.batch->size() > 64 ? reinterpret_cast<std::vector<Vec3f>*>(&data.batch->origins) : &data.batch->directions;

				isRunning = true;

				return (true);
			}

			void execute() override
			{
				SortingPack pack;
				{ // lock scope
					std::unique_lock<std::mutex> lock(guard);
					sleepCtrl.wait(lock, [this, &pack]()
						{
							return (getNextPack(pack)
								|| !isRunning);
						});
				}

				if (!isRunning)
					return;

				while (true)
				{
					if (pack.end - pack.start < pack.target - 1)
					{
						long pivot = 0;
						while (pivot < pack.start)
							pivot += pack.target;

						if (pack.end - pack.start < pivot - pack.start)
						{
							if (pack.target == 4096)
							{
								SortingPack newPack;
								newPack.start = pack.start;
								newPack.end = pack.end;
								newPack.target = 64;
								newPack.order = &data.batch->directions;
								pushPack(newPack);
							}

							if (!getNextPack(pack))
								break;
							continue;
						}
					}

					// Insertion sort when subarray small enough.
					if (pack.end - pack.start < M)
					{
						uint8_t axis = pickLargestAxis(*pack.order, pack.start, pack.end);

						for (int32_t j = pack.start + 1; j <= pack.end; j++)
						{
							int32_t i;
							float a = (*pack.order)[j][axis];
							//NRay aValue = owner.batch[j];
							for (i = j - 1; i >= pack.start; i--)
							{
								if ((*pack.order)[i][axis] <= a)
									break;
								data.batch->swap(i + 1, i);
							}
							//owner.batch.set(aValue, i + 1);
						}

						if (!getNextPack(pack))
							break;
					}
					else
					{
						uint8_t axis = pickLargestAxis(*pack.order, pack.start, pack.end);

						// l = 1;
						// ir = n;
						int32_t k = (pack.start + pack.end) >> 1; // find pivot point
						data.batch->swap(k, pack.start + 1);
						if ((*pack.order)[pack.start][axis] > (*pack.order)[pack.end][axis]) // if first point superior at last point swap point
							data.batch->swap(pack.start, pack.end);
						if ((*pack.order)[pack.start + 1][axis] > (*pack.order)[pack.end][axis]) // if first pont + 1 superior to last point, swap
							data.batch->swap(pack.start + 1, pack.end);
						if ((*pack.order)[pack.start][axis] > (*pack.order)[pack.start + 1][axis]) //  if first point superior to first point + 1, swap
							data.batch->swap(pack.start, pack.start + 1);

						int32_t i = pack.start + 1;
						int32_t j = pack.end;
						float a = (*pack.order)[pack.start + 1][axis];

						//NRay aValue = owner.batch[pack.start + 1];
						for (;;)
						{
							do
								i++;
							while ((*pack.order)[i][axis] < a);

							do
								j--;
							while ((*pack.order)[j][axis] > a);

							if (j < i)
								break;
							data.batch->swap(i, j);
						}
						data.batch->swap(pack.start + 1, j); // Insert partitioning element in both arrays.
						//data.batch->set(aValue, j);

						if (pack.end - i + 1 >= j - pack.start)
						{
							SortingPack newPack;
							newPack.start = i;
							newPack.end = pack.end;
							newPack.target = pack.target;
							newPack.order = pack.order;
							pushPack(newPack);
							pack.end = j - 1;
						}
						else
						{
							SortingPack newPack;
							newPack.start = pack.start;
							newPack.end = j;
							newPack.target = pack.target;
							newPack.order = pack.order;
							pushPack(newPack);
							pack.start = i;
						}
						sleepCtrl.notify_one();
					}
				}

				isRunning = false;
				sleepCtrl.notify_all();
			}

			void postExecute() override
			{
				sortingPacks.clear();
			}

			uint8_t pickLargestAxis(const std::vector<Vec3f> &ar, uint32_t firstIndex, uint32_t lastIndex) const
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

			bool getNextPack(SortingPack &pack)
			{
				if (pullIndex >= pushIndex)
					return (false);

				uint32_t offset = pullIndex.fetch_add(1);
				offset = offset % sortingPacks.size();
				pack = sortingPacks[offset];
				return (true);
			}

			void pushPack(const SortingPack &pack)
			{
				uint32_t offset = pushIndex.fetch_add(1);
				offset = offset % sortingPacks.size();
				sortingPacks[offset] = pack;
			}

		private:
			static constexpr long M = 7;
			static constexpr long NStack = 50;

			Data data;

			bool isRunning;

			std::mutex guard;
			std::condition_variable sleepCtrl;

			std::atomic<uint32_t> pushIndex;
			std::atomic<uint32_t> pullIndex;
			std::vector<SortingPack> sortingPacks;
		};
	}
}