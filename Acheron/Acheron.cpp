#include "pch.h"
#include "Acheron.h"

#include "Atlas/Bound.h"
#include "Atlas/Hitable.h"
#include "Atlas/Chronometer.h"
#include "Atlas/Tools.h"
#include "Atlas/rendering/Ray.h"
#include "Atlas/rendering/HitRecord.h"
#include <algorithm>
#include <iostream>

#include "Atlas/rendering/HitRecord.h"

#include <glm/gtc/constants.hpp>

#include "LocalBins.h"
#include "CompactRay.h"
#include "BoundingCone.h"
#include "Telemetry.h"
#include "utils.h"
#include "Cone.h"
#include "BinSize.h"

DEFINE_SECTOR(tlyBuildingAcc, "Acheron/Building acceleration structure");
DEFINE_SECTOR(tlyExecution, "Acheron/Execution");
DEFINE_SECTOR(tlyIteration, "Acheron/Iteration");
DEFINE_SECTOR(tlyBuildingBatches, "Acheron/Building batches");
DEFINE_SECTOR(tlyProcessRays, "Acheron/ProcessRays");
DEFINE_SECTOR(tlyExtractBatch, "Acheron/ProcessRays/Extract Batch");
DEFINE_SECTOR(tlySortRays, "Acheron/ProcessRays/Sort rays");
DEFINE_SECTOR(tlyTraceRays, "Acheron/ProcessRays/Trace rays");

class GenerateFirstRaysTask : public Task
{
	Acheron &owner;

	std::atomic<uint64_t> zOrderPos = 0;

	uint64_t zOrderMax = 0;
	uint64_t zOrderStep = 516;

public:
	GenerateFirstRaysTask(Acheron &owner)
		: owner(owner), zOrderMax(posToZOrderIndex(owner.width, owner.height))
	{}

	void execute() override
	{
		thread_local LocalBins localBins(owner.bins);

		while (true)
		{
			const uint64_t offset = zOrderPos.fetch_add(zOrderStep);
			if (offset >= zOrderMax)
				break;

			for (uint64_t i = offset; i < offset + zOrderStep; i++)
			{
				uint32_t x;
				uint32_t y;
				zOrderIndexToPos(i, x, y);
				if (x < owner.width && y < owner.height)
				{
					for (uint32_t s = 0; s < owner.currentSampleTarget; s++)
					{
						const float u = (static_cast<float>(x) + atlas::Tools::rand()) / owner.width;
						const float v = (static_cast<float>(y) + atlas::Tools::rand()) / owner.height;

						const atlas::rendering::Ray ray = owner.camera->getRay(u, v);
						CompactRay compactRay(ray, x + y * owner.width, s, owner.tmin);

						localBins.feed(ray, compactRay);
					}
				}
			}
		}
		localBins.flush();
	}
};

class ExtractBatchTask : public Task
{
	Acheron &owner;

	std::string name;
	void *file;
	void *mapping;
	CompactRay *buffer;

	std::atomic<uint32_t> index = 0;
	uint32_t size = 0;
	const uint32_t maxRayPerPass = 512;

public:
	ExtractBatchTask(Acheron &owner, const std::string &filename, uint32_t size = BIN_SIZE)
		: owner(owner), name(filename), size(size)
	{
		const size_t byteSize = sizeof(uint32_t) + size * sizeof(CompactRay);
		
		uint32_t flags = OPEN_EXISTING;

		file = CreateFileA(filename.c_str(), GENERIC_READ, 0, nullptr, flags, FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_TEMPORARY | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
		mapping = CreateFileMappingA(file, nullptr, PAGE_READONLY, 0, byteSize, nullptr);
		buffer = (CompactRay *)MapViewOfFile(mapping, FILE_MAP_READ, 0, 0, byteSize);

		owner.batch.resize(size);
	}

	~ExtractBatchTask()
	{
		UnmapViewOfFile(buffer);
		CloseHandle(mapping);
		CloseHandle(file);

		// DeleteFileA(name.c_str());
	}

	void execute() override
	{
		while (true)
		{
			uint32_t offset = index.fetch_add(maxRayPerPass);
			if (offset >= size)
				break;

			uint32_t end = std::min(offset + maxRayPerPass, (uint32_t)size);
			for (uint32_t i = offset; i < end; i++)
			{
				NRay r;
				buffer[i].extract(r);

				owner.batch.origins[i] = r.origin;
				owner.batch.directions[i] = r.dir;
				owner.batch.colors[i] = r.weight;
				owner.batch.pixelIDs[i] = r.pixelID;
				owner.batch.sampleIDs[i] = r.sampleID;
				owner.batch.depths[i] = r.depth;
				owner.batch.tNears[i] = r.tNear;
			}
		}
	}
};

class QuickSortTask : public Task
{
	struct Pack
	{
		std::vector<glm::vec3> *order;
		uint32_t firstIndex;
		uint32_t size;
		uint32_t target;
	};

	struct SortingPack
	{
		int32_t start;
		int32_t end;

		int32_t target;
		std::vector<glm::vec3> *order;
	};

	Acheron &owner;

	std::atomic<uint32_t> pushIndex;
	std::atomic<uint32_t> pullIndex;
	std::vector<SortingPack> sortingPacks;

	const long M = 7;
	const long NSTACK = 50;

public:
	QuickSortTask(Acheron &owner)
		: owner(owner)
	{
		pushIndex = 1;
		pullIndex = 0;

		sortingPacks.resize(owner.batch.size());

		sortingPacks[0].start = 0;
		sortingPacks[0].end = owner.batch.size() - 1;

		sortingPacks[0].target = owner.batch.size() > 64 ? 4096 : 64;
		sortingPacks[0].order = owner.batch.size() > 64 ? &owner.batch.origins : &owner.batch.directions;
	}

	uint8_t pickLargestAxis(const std::vector<glm::vec3> &ar, uint32_t firstIndex, uint32_t lastIndex) const
	{
		glm::vec2 rangeX(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
		glm::vec2 rangeY(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
		glm::vec2 rangeZ(std::numeric_limits<float>::max(), std::numeric_limits<float>::min());
		for (uint32_t i = firstIndex; i <= lastIndex; i++)
		{
			rangeX[0] = std::min(rangeX[0], owner.batch.origins[i].x);
			rangeX[1] = std::max(rangeX[1], owner.batch.origins[i].x);

			rangeY[0] = std::min(rangeY[0], owner.batch.origins[i].y);
			rangeY[1] = std::max(rangeY[1], owner.batch.origins[i].y);

			rangeZ[0] = std::min(rangeZ[0], owner.batch.origins[i].z);
			rangeZ[1] = std::max(rangeZ[1], owner.batch.origins[i].z);
		}

		const float lengthX = rangeX[1] - rangeX[0];
		const float lengthY = rangeY[1] - rangeY[0];
		const float lengthZ = rangeZ[1] - rangeZ[0];

		return (lengthX < lengthY ? (lengthY < lengthZ ? 2 : 1) : (lengthX < lengthZ ? 2 : 0));
	}

	void quicksort()
	{
		int32_t start = 0;
		int32_t end = static_cast<int32_t>(owner.batch.origins.size()) - 1;

		int32_t target;
		std::vector<glm::vec3> *order;

		std::vector<Pack> stack;
		int32_t jstack = 0;
		float a, b, temp;

		if (owner.batch.origins.size() > 4096)
		{
			target = 4096;
			order = &owner.batch.origins;
		}
		else
		{
			target = 64;
			order = &owner.batch.directions;
		}


		stack.resize(NSTACK);
		for (;;)
		{
			if (end - start < target - 1)
			{
				long pivot = 0;
				while (pivot < start)
					pivot += target;

				if (end - start < pivot - start)
				{
					if (target == 4096)
					{
						target = 64;
						order = &owner.batch.directions;
						continue;
					}

					if (!jstack)
						return;

					jstack -= 1;
					start = stack[jstack].firstIndex;
					end = start + stack[jstack].size - 1;
					target = stack[jstack].target;
					order = stack[jstack].order;

					continue;
				}
			}

			// Insertion sort when subarray small enough.
			if (end - start < M)
			{
				uint8_t axis = pickLargestAxis(*order, start, end);

				for (int32_t j = start + 1; j <= end; j++)
				{
					int32_t i;
					a = (*order)[j][axis];
					NRay aValue = owner.batch[j];
					for (i = j - 1; i >= start; i--)
					{
						if ((*order)[i][axis] <= a) break;
						owner.batch.swap(i + 1, i);
					}
					owner.batch.set(aValue, i + 1);
				}
				if (!jstack)
				{
					return;
				}
				jstack -= 1;
				start = stack[jstack].firstIndex;
				end = start + stack[jstack].size - 1;
				target = stack[jstack].target;
				order = stack[jstack].order;
			}
			else
			{
				uint8_t axis = pickLargestAxis(*order, start, end);

				// l = 1;
				// ir = n;
				int32_t k = (start + end) >> 1; // find pivot point
				owner.batch.swap(k, start + 1);
				if ((*order)[start][axis] > (*order)[end][axis]) // if first point superior at last point swap point
					owner.batch.swap(start, end);
				if ((*order)[start + 1][axis] > (*order)[end][axis]) // if first pont + 1 superior to last point, swap
					owner.batch.swap(start + 1, end);
				if ((*order)[start][axis] > (*order)[start + 1][axis]) //  if first point superior to first point + 1, swap
					owner.batch.swap(start, start + 1);

				int32_t i = start + 1;
				int32_t j = end;
				a = (*order)[start + 1][axis];
				NRay aValue = owner.batch[start + 1];
				for (;;)
				{
					do
						i++;
					while ((*order)[i][axis] < a);

					do
						j--;
					while ((*order)[j][axis] > a);

					if (j < i)
						break;
					owner.batch.swap(i, j);
				}

				owner.batch.swap(start + 1, j); // Insert partitioning element in both arrays.
				owner.batch.set(aValue, j);

				// Push pointers to larger subarray on stack, process smaller subarray immediately.
				if (jstack > NSTACK)
					printf("NSTACK too small in sort2.");
				if (end - i + 1 >= j - start)
				{
					stack[jstack].size = end + 1 - i;
					stack[jstack].firstIndex = i;
					stack[jstack].order = order;
					stack[jstack].target = target;
					end = j - 1;
				}
				else
				{
					stack[jstack].size = j - start;
					stack[jstack].firstIndex = start;
					start = i;
				}
				stack[jstack].order = order;
				stack[jstack].target = target;
				jstack += 1;
			}
		}
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

	void expExecute(DuplicateThread duplicateThread) override
	{
		SortingPack pack;
		if (!getNextPack(pack))
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
						newPack.order = &owner.batch.directions;
						pushPack(newPack);
					}

					if (!getNextPack(pack))
						return;
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
					NRay aValue = owner.batch[j];
					for (i = j - 1; i >= pack.start; i--)
					{
						if ((*pack.order)[i][axis] <= a) break;
						owner.batch.swap(i + 1, i);
					}
					owner.batch.set(aValue, i + 1);
				}

				if (!getNextPack(pack))
					return;
			}
			else
			{
				uint8_t axis = pickLargestAxis(*pack.order, pack.start, pack.end);

				// l = 1;
				// ir = n;
				int32_t k = (pack.start + pack.end) >> 1; // find pivot point
				owner.batch.swap(k, pack.start + 1);
				if ((*pack.order)[pack.start][axis] > (*pack.order)[pack.end][axis]) // if first point superior at last point swap point
					owner.batch.swap(pack.start, pack.end);
				if ((*pack.order)[pack.start + 1][axis] > (*pack.order)[pack.end][axis]) // if first pont + 1 superior to last point, swap
					owner.batch.swap(pack.start + 1, pack.end);
				if ((*pack.order)[pack.start][axis] > (*pack.order)[pack.start + 1][axis]) //  if first point superior to first point + 1, swap
					owner.batch.swap(pack.start, pack.start + 1);

				int32_t i = pack.start + 1;
				int32_t j = pack.end;
				float a = (*pack.order)[pack.start + 1][axis];

				NRay aValue = owner.batch[pack.start + 1];
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
					owner.batch.swap(i, j);
				}
				owner.batch.swap(pack.start + 1, j); // Insert partitioning element in both arrays.
				owner.batch.set(aValue, j);

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
				duplicateThread();
			}
		}
	}
};

class TraceRaysTask : public Task
{
	Acheron &owner;

	std::atomic<uint32_t> traceRaysIndex;
	const uint32_t maxConeSize = 64;
	const uint32_t maxPackSize = maxConeSize * 4;

public:
	TraceRaysTask(Acheron &owner)
		: owner(owner)
	{}

	BoundingCone packRaysInCone(uint32_t startingIndex, uint32_t size)
	{
		glm::vec3 dir(0);
		glm::vec3 pointOnAxe(0);
		for (uint32_t j = 0; j < size; j++)
		{
			dir += owner.batch.directions[startingIndex + j];
			pointOnAxe += owner.batch.origins[startingIndex + j];
		}
		dir *= 1.f / size;
		pointOnAxe *= 1.f / size;

		float dot = 1.f;
		glm::vec3 ref;
		float longestDist = std::numeric_limits<float>::min();
		for (uint32_t j = 0; j < size; j++)
		{
			float dirDot = glm::dot(dir, owner.batch.directions[startingIndex + j]);
			dirDot = std::min(dirDot, 0.90f);

			float offset = glm::dot(dir, owner.batch.origins[startingIndex + j] - pointOnAxe);
			glm::vec3 h = pointOnAxe + dir * offset;
			float hypLength = glm::length(h) / glm::dot(glm::normalize(h - owner.batch.origins[startingIndex + j]), -owner.batch.directions[startingIndex + j]);
			float dist = dirDot * hypLength;

			if (offset + dist > longestDist)
			{
				ref = owner.batch.origins[startingIndex + j];
				longestDist = offset + dist;
				dot = dirDot;
			}
		}

		BoundingCone cone;
		cone.origin = pointOnAxe - dir * longestDist;
		cone.dir = dir;
		cone.dot = dot;// glm::dot(glm::normalize(ref - cone.origin), dir);
		for (uint32_t j = 0; j < size; j++)
		{
			cone.tmin = std::min(cone.tmin, glm::dot(cone.dir, (owner.batch.origins[startingIndex + j] + owner.tmin * owner.batch.directions[startingIndex + j]) - cone.origin));
			cone.tmax = std::max(cone.tmax, glm::dot(cone.dir, (owner.batch.origins[startingIndex + j] + owner.tmax * owner.batch.directions[startingIndex + j]) - cone.origin));
		}

#if 1 // test if all rays are inside the cone
		for (uint32_t j = 0; j < size; j++)
		{
			if (glm::dot(glm::normalize(owner.batch.origins[startingIndex + j] - cone.origin), dir) < cone.dot)
			{
				float a = glm::dot(glm::normalize(owner.batch.origins[startingIndex + j] - cone.origin), dir);
				std::cout << "cone mismatch" << std::endl;
				break;
			}

			if (glm::dot(owner.batch.directions[startingIndex + j], dir) < cone.dot)
			{
				float a = glm::dot(owner.batch.directions[startingIndex + j], dir);
				std::cout << "cone mismatch" << std::endl;
				break;
			}
		}
#endif

		return (cone);
	}


	void execute() override
	{
		while (true)
		{
			uint32_t index = traceRaysIndex.fetch_add(maxPackSize);
			if (index >= owner.batch.size())
				break;

			uint32_t size = std::min(maxPackSize, owner.batch.size() - index);
			for (uint32_t i = 0; i < size; i += maxConeSize)
			{
				Cone cone;
				cone.nbrRays = std::min(maxConeSize, size - i);
				cone.bound = packRaysInCone(index + i, cone.nbrRays);
				for (uint32_t j = 0; j < cone.nbrRays; j++)
				{
					cone.r[j] = atlas::rendering::Ray(owner.batch.origins[index + i + j], owner.batch.directions[index + i + j]);
					cone.h[j].reset();
					cone.h[j].t = owner.tmax;
					//owner.accelerations.front().hit(cone.r[j], owner.tmin, owner.tmax, cone.h[j]);
				}

				owner.accelerations.front().hit(cone, owner.tmin, owner.tmax);

				//for (uint32_t j = 0; j < cone.nbrRays; j++)
				//{
				//	hitPoints[index + i + j] = cone.h[j];
				//}

				memcpy(&owner.hitPoints.data()[index + i], cone.h, cone.nbrRays * sizeof(HitRecord));
			}
		}
	}
};

class RadixSortTask : public Task
{
	Acheron &owner;

public:
	RadixSortTask(Acheron &owner)
		: owner(owner)
	{}
};

Acheron::Acheron(const Scene &s, uint32_t w, uint32_t h, uint32_t nbrS)
	: scene(s)
	, width(w), height(h), maxSample(nbrS)
	, bins(batchManager)
{
	SINGLE_TIMED_SCOPE(tlyBuildingAcc);

	output.resize(w * h);

	bool isFirst = true;
	Bound bound;
	std::vector<const Hitable *> elements;
	for (auto &shape : scene.getShapes())
	{
		elements.emplace_back(shape.get());
		if (isFirst)
		{
			bound = shape->getBound();
			isFirst = false;
		}
		else
			bound += shape->getBound();
	}
	accelerations.reserve(scene.getShapes().size() * 2);
	accelerations.emplace_back();
	accelerations.back().feed(elements, bound, accelerations);
}

Acheron::~Acheron()
{
	std::cout << "Stopping" << std::endl;

	isRendering = false;
	scheduler.stop();
}

void Acheron::launch(const atlas::Camera &c)
{
	isRendering = true;

	{
		SINGLE_TIMED_SCOPE(tlyExecution);
		camera = &c;

		scheduler.startThreads(context.threadCount);

		uint32_t step = context.firstIterationSampleCount;
		for (uint32_t i = 0; i < maxSample; i += step)
		{
			MULTIPLE_TIMED_SCOPE(tlyIteration);

			currentSampleTarget = i + step <= maxSample ? step : i + step - maxSample;
			runOneIteration();
			step = std::max(step * 2, context.maxSamplePerIteration);
			break;
		}
	}

	PRINT_TELEMETRY_REPORT();
	
	scheduler.stop();
}

void Acheron::runOneIteration()
{
	{
		MULTIPLE_TIMED_SCOPE(tlyBuildingBatches);

		// generate camera rays
		bins.open();
		GenerateFirstRaysTask task(*this);
		scheduler.dispatch(task);
		bins.unmap();
	}

	// processRays
	processRays();

	// update adaptive sampler
	// update cache points
	// save image files & checkpoint
}

void Acheron::processRays()
{
	MULTIPLE_TIMED_SCOPE(tlyProcessRays);

	while (true)
	{
		{
			MULTIPLE_TIMED_SCOPE(tlyExtractBatch);
			BinFile *bin = nullptr;
			std::string batchName = batchManager.popBatchName();
			if (!batchName.empty())
			{
#if 0	
				BinFile::extract(batchName, batch);
#else
				ExtractBatchTask task(*this, batchName);
				scheduler.dispatch(task);
#endif
			}
#if 1
			else if ((bin = bins.getBin()))
			{
				ExtractBatchTask task(*this, bin->getFilename(), bin->getSize());
				scheduler.dispatch(task);
				bin->close();
			}
			else
				return;
#else
			else if (!bins.purge(batch))
				return;
#endif
		}

		{
			MULTIPLE_TIMED_SCOPE(tlySortRays);
			QuickSortTask task(*this);
#if 1
			scheduler.expDispatch(task);
#else
			task.quicksort();
#endif
		}

		// traverse rays
		{
			MULTIPLE_TIMED_SCOPE(tlyTraceRays);
			hitPoints.resize(batch.size());
#if 0
			for (uint32_t i = 0; i < batch.origins.size(); i++)
			{
				atlas::rendering::Ray ray(batch.origins[i], batch.directions[i]);

				hitPoints[i].reset();
				accelerations.front().hit(ray, tmin, tmax, hitPoints[i]);
			}
#else
			TraceRaysTask task(*this);
			scheduler.dispatch(task);
#endif
		}

		//// sort hit point
		//std::sort(hitPoints.begin(), hitPoints.end(), [](const HitRecord &h1, const HitRecord &h2) // temporary
		//	{
		//		if (!h1.material)
		//			return (true);
		//		else if (!h2.material)
		//			return (false);
		//		return ((uint64_t)h1.material < (uint64_t)h2.material);
		//	});

		{
			bins.map();
			LocalBins localBins(bins);
			for (uint32_t i = 0; i < batch.size(); i++)
			{
				glm::vec3 attenuation;
				atlas::rendering::Ray out;
				atlas::rendering::Ray ray(batch.origins[i], batch.directions[i]);

				atlas::rendering::HitRecord hit;
				hit.t = hitPoints[i].t;
				hit.p = hitPoints[i].p;
				hit.uv = hitPoints[i].uv;
				hit.normal = hitPoints[i].normal;
				hit.material = hitPoints[i].material;

				if (batch.depths[i] < maxDepth && hitPoints[i].hit && hitPoints[i].material->scatter(ray, hit, attenuation, out))
				{
					CompactRay compactRay(out, batch.colors[i] * attenuation, batch.pixelIDs[i], batch.sampleIDs[i], batch.depths[i] + 1, batch.tNears[i]);
					localBins.feed(ray, compactRay);

					//output.image[batch.pixelIDs[i]] += attenuation * 0.25f;
				}
				else
				{
					glm::vec3 direction = glm::normalize(ray.dir);
					float t = 0.5f * (direction.y + 1);
					glm::vec3 sky = (1 - t) * glm::vec3(1, 1, 1) + t * glm::vec3(0.5, 0.7, 1);
					output.image[batch.pixelIDs[i]] += batch.colors[i] * sky * 0.25f;
				}
			}
			localBins.flush();
			bins.unmap();
		}
	}
}