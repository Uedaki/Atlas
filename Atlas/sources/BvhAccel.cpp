#include "atlas/primitives/BvhAccel.h"

#include "atlas/core/Bounds.h"
#include "atlas/core/Points.h"
#include "atlas/core/ConeBoxIntersection.h"

using namespace atlas;

namespace atlas
{
	struct BvhPrimitiveInfo
	{
		BvhPrimitiveInfo() = default;
		BvhPrimitiveInfo(uint32_t primitiveNbr, const Bounds3f &bounds)
			: primitiveNbr(primitiveNbr)
			, bounds(bounds)
			, centroid(bounds.min * 0.5f + bounds.max * 0.5f)
		{}

		uint32_t primitiveNbr = 0;
		Bounds3f bounds;
		Point3f centroid;
	};

	struct BvhBuildNode
	{
		void initLeaf(int32_t first, int32_t n, const Bounds3f &b)
		{
			firstPrimOffset = first;
			nPrimitives = n;
			bounds = b;

			children[0] = nullptr;
			children[1] = nullptr;
		}

		void initInterior(int32_t axis, BvhBuildNode *c0, BvhBuildNode *c1)
		{
			children[0] = c0;
			children[1] = c1;
			bounds = expand(c0->bounds, c1->bounds);
			splitAxis = axis;
			nPrimitives = 0;
		}

		int32_t splitAxis = 0;
		int32_t firstPrimOffset = 0;
		int32_t nPrimitives = 0;
		Bounds3f bounds;
		BvhBuildNode *children[2] = {nullptr, nullptr};
	};

	struct LinearBvhNode
	{
		Bounds3f bounds;
		union
		{
			int32_t primitiveOffset = 0;
			int32_t secondChildOffset;
		};
		uint16_t nPrimitives = 0;
		uint8_t axis = 0;
	};

	struct BucketInfo
	{
		int32_t count = 0;
		Bounds3f bounds;
	};
}

BvhAccel::BvhAccel(const std::vector<std::shared_ptr<Primitive>> &p)
	: primitives(p)
{
	if (primitives.empty())
		return;

	std::vector<BvhPrimitiveInfo> primitiveInfo(primitives.size());
	for (uint32_t i = 0; i < primitives.size(); i++)
		primitiveInfo[i] = { i, primitives[i]->worldBound() };

	int32_t totalNodes = 0;
	BvhBuildNode *root = nullptr;
	std::vector<std::shared_ptr<Primitive>> orderedPrims;
	root = recursiveBuild(primitiveInfo, 0, static_cast<uint32_t>(primitives.size()), totalNodes, orderedPrims);
	primitives.swap(orderedPrims);
	primitiveInfo.resize(0);

	int32_t offset = 0;
	nodes = new LinearBvhNode[totalNodes];
	flattenBvhTree(root, offset);
	CHECK(totalNodes == offset);
}

BvhBuildNode *BvhAccel::recursiveBuild(std::vector<BvhPrimitiveInfo> &primitiveInfo,
	int32_t start, int32_t end, int32_t &totalNodes,
	std::vector<std::shared_ptr<Primitive>> &orderedPrims)
{
	BvhBuildNode *node = new BvhBuildNode;
	totalNodes++;
	// Compute bounds of all primitives in BVH node
	Bounds3f bounds;
	for (int i = start; i < end; ++i)
		bounds = expand(bounds, primitiveInfo[i].bounds);
	int nPrimitives = end - start;
	if (nPrimitives == 1) {
		// Create leaf _BVHBuildNode_
		int32_t firstPrimOffset = static_cast<int32_t>(orderedPrims.size());
		for (int i = start; i < end; ++i) {
			int primNum = primitiveInfo[i].primitiveNbr;
			orderedPrims.push_back(primitives[primNum]);
		}
		node->initLeaf(firstPrimOffset, nPrimitives, bounds);
		return node;
	}
	else {
		// Compute bound of primitive centroids, choose split dimension _dim_
		Bounds3f centroidBounds;
		for (int i = start; i < end; ++i)
			centroidBounds = expand(centroidBounds, primitiveInfo[i].centroid);
		int dim = centroidBounds.maxExtent();

		// Partition primitives into two sets and build children
		int32_t mid = (start + end) / 2;
		if (centroidBounds.max[dim] == centroidBounds.min[dim]) {
			// Create leaf _BVHBuildNode_
			int32_t firstPrimOffset = static_cast<int32_t>(orderedPrims.size());
			for (int i = start; i < end; ++i) {
				int primNum = primitiveInfo[i].primitiveNbr;
				orderedPrims.push_back(primitives[primNum]);
			}
			node->initLeaf(firstPrimOffset, nPrimitives, bounds);
			return node;
		}
		else {
			// Partition primitives using approximate SAH
			if (nPrimitives <= 2) {
				// Partition primitives into equally-sized subsets
				mid = (start + end) / 2;
				std::nth_element(&primitiveInfo[start], &primitiveInfo[mid],
					&primitiveInfo[end - 1] + 1,
					[dim](const BvhPrimitiveInfo &a,
						const BvhPrimitiveInfo &b) {
							return a.centroid[dim] <
								b.centroid[dim];
					});
			}
			else {
				// Allocate _BucketInfo_ for SAH partition buckets
				constexpr int32_t nBuckets = 12;
				BucketInfo buckets[nBuckets];

				// Initialize _BucketInfo_ for SAH partition buckets
				for (int i = start; i < end; ++i) {
					int32_t b = static_cast<int32_t>(nBuckets *
						centroidBounds.offset(
							primitiveInfo[i].centroid)[dim]);
					if (b == nBuckets) b = nBuckets - 1;
					buckets[b].count++;
					buckets[b].bounds =
						expand(buckets[b].bounds, primitiveInfo[i].bounds);
				}

				// Compute costs for splitting after each bucket
				Float cost[nBuckets - 1];
				for (int i = 0; i < nBuckets - 1; ++i) {
					Bounds3f b0, b1;
					int count0 = 0, count1 = 0;
					for (int j = 0; j <= i; ++j) {
						if (buckets[j].count != 0)
						{
							b0 = expand(b0, buckets[j].bounds);
							count0 += buckets[j].count;
						}
					}
					for (int j = i + 1; j < nBuckets; ++j) {
						if (buckets[j].count != 0)
						{
							b1 = expand(b1, buckets[j].bounds);
							count1 += buckets[j].count;
						}
					}
					cost[i] = 1 +
						(count0 * b0.surfaceArea() +
							count1 * b1.surfaceArea()) /
						bounds.surfaceArea();
				}

				// Find bucket to split at that minimizes SAH metric
				Float minCost = cost[0];
				int minCostSplitBucket = 0;
				for (int i = 1; i < nBuckets - 1; ++i) {
					if (cost[i] < minCost) {
						minCost = cost[i];
						minCostSplitBucket = i;
					}
				}

				// Either create leaf or split primitives at selected SAH
				// bucket
				Float leafCost = static_cast<Float>(nPrimitives);
				if (nPrimitives > 2 || minCost < leafCost) {
					BvhPrimitiveInfo *pmid = std::partition(
						&primitiveInfo[start], &primitiveInfo[end - 1] + 1,
						[=](const BvhPrimitiveInfo &pi) {
							int32_t b = static_cast<int32_t>(nBuckets *
								centroidBounds.offset(pi.centroid)[dim]);
							if (b == nBuckets) b = nBuckets - 1;
							return b <= minCostSplitBucket;
						});
					mid = static_cast<int32_t>(pmid - &primitiveInfo[0]);
				}
				else {
					// Create leaf _BVHBuildNode_
					int32_t firstPrimOffset = static_cast<int32_t>(orderedPrims.size());
					for (int i = start; i < end; ++i) {
						int primNum = primitiveInfo[i].primitiveNbr;
						orderedPrims.push_back(primitives[primNum]);
					}
					node->initLeaf(firstPrimOffset, nPrimitives, bounds);
					return node;
				}
			}
			node->initInterior(dim,
				recursiveBuild(primitiveInfo, start, mid,
					totalNodes, orderedPrims),
				recursiveBuild(primitiveInfo, mid, end,
					totalNodes, orderedPrims));
		}
	}
	return node;
}

int32_t BvhAccel::flattenBvhTree(BvhBuildNode *node, int32_t &offset)
{
	LinearBvhNode *linearNode = &nodes[offset];
	linearNode->bounds = node->bounds;
	int32_t myOffset = offset++;
	if (node->nPrimitives > 0)
	{
		CHECK(!node->children[0] && !node->children[1]);
		linearNode->primitiveOffset = node->firstPrimOffset;
		linearNode->nPrimitives = node->nPrimitives;
	}
	else
	{
		linearNode->axis = node->splitAxis;
		linearNode->nPrimitives = 0;
		flattenBvhTree(node->children[0], offset);
		linearNode->secondChildOffset = flattenBvhTree(node->children[1], offset);
	}
	return (myOffset);
}

bool BvhAccel::intersect(const Ray &r, SurfaceInteraction &intersection) const
{
	if (!nodes)
		return (false);

	bool hit = false;
	Vec3f invDir(1.f / r.dir.x, 1.f / r.dir.y, 1.f / r.dir.z);
	int8_t dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

	int32_t toVisitOffset = 0;
	int32_t currentNodeIndex = 0;
	int32_t nodesToVisit[64];
	while (true)
	{
		const LinearBvhNode *node = &nodes[currentNodeIndex];
		if (node->bounds.intersectP(r, invDir, dirIsNeg))
		{
			if (node->nPrimitives > 0)
			{
				for (int32_t i = 0; i < node->nPrimitives; i++)
				{
					if (primitives[node->primitiveOffset + i]->intersect(r, intersection))
						hit = true;
				}
				if (toVisitOffset == 0)
					break;
#if 0
				currentNodeIndex = nodesToVisit[--toVisitOffset];
#else
				currentNodeIndex = nodesToVisit[toVisitOffset - 1];
				toVisitOffset--;
#endif
			}
			else
			{
				if (dirIsNeg[node->axis])
				{
					nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
					currentNodeIndex = node->secondChildOffset;
				}
				else
				{
					nodesToVisit[toVisitOffset++] = node->secondChildOffset;
					currentNodeIndex = currentNodeIndex + 1;
				}
			}
		}
		else
		{
			if (toVisitOffset == 0)
				break;
#if 0
			currentNodeIndex = nodesToVisit[--toVisitOffset];
#else
			currentNodeIndex = nodesToVisit[toVisitOffset - 1];
			toVisitOffset--;
#endif
		}
	}
	return (hit);
}

bool BvhAccel::intersectP(const Ray &r) const
{
	bool hit = false;
	Vec3f invDir(1.f / r.dir.x, 1.f / r.dir.y, 1.f / r.dir.z);
	int8_t dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

	int32_t toVisitOffset = 0;
	int32_t currentNodeIndex = 0;
	int32_t nodesToVisit[64];
	while (true)
	{
		const LinearBvhNode *node = &nodes[currentNodeIndex];
		if (node->bounds.intersectP(r, invDir, dirIsNeg))
		{
			if (node->nPrimitives > 0)
			{
				for (int32_t i = 0; i < node->nPrimitives; i++)
				{
					if (primitives[node->primitiveOffset + i]->intersectP(r))
						hit = true;
				}

				if (toVisitOffset == 0)
					break;
#if 0
				currentNodeIndex = nodesToVisit[--toVisitOffset];
#else
				currentNodeIndex = nodesToVisit[toVisitOffset - 1];
				toVisitOffset--;
#endif
			}
			else
			{
				if (dirIsNeg[node->axis])
				{
					nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
					currentNodeIndex = node->secondChildOffset;
				}
				else
				{
					nodesToVisit[toVisitOffset++] = node->secondChildOffset;
					currentNodeIndex = currentNodeIndex + 1;
				}
			}
		}
		else
		{
			if (toVisitOffset == 0)
				break;
#if 0
			currentNodeIndex = nodesToVisit[--toVisitOffset];
#else
			currentNodeIndex = nodesToVisit[toVisitOffset - 1];
			toVisitOffset--;
#endif
		}
	}
	return (hit);
}

void BvhAccel::intersect(const Payload &p, std::vector<SurfaceInteraction> &it, std::vector<Float> &tmax) const
{
	bool hit = false;
	Vec3f invDir(1.f / p.cone.dir.x, 1.f / p.cone.dir.y, 1.f / p.cone.dir.z);
	int8_t dirIsNeg[3] = { invDir.x < 0, invDir.y < 0, invDir.z < 0 };

	int32_t toVisitOffset = 0;
	int32_t currentNodeIndex = 0;
	int32_t nodesToVisit[64];
	while (true)
	{
		const LinearBvhNode *node = &nodes[currentNodeIndex];
		if (atlas::intersectP(node->bounds, p.cone))
		{
			if (node->nPrimitives > 0)
			{
				for (int32_t i = 0; i < node->nPrimitives; i++)
				{
					primitives[node->primitiveOffset + i]->intersect(p, it, tmax);
				}
				if (toVisitOffset == 0)
					break;
#if 0
				currentNodeIndex = nodesToVisit[--toVisitOffset];
#else
				currentNodeIndex = nodesToVisit[toVisitOffset - 1];
				toVisitOffset--;
#endif
			}
			else
			{
				if (dirIsNeg[node->axis])
				{
					nodesToVisit[toVisitOffset++] = currentNodeIndex + 1;
					currentNodeIndex = node->secondChildOffset;
				}
				else
				{
					nodesToVisit[toVisitOffset++] = node->secondChildOffset;
					currentNodeIndex = currentNodeIndex + 1;
				}
			}
		}
		else
		{
			if (toVisitOffset == 0)
				break;
#if 0
			currentNodeIndex = nodesToVisit[--toVisitOffset];
#else
			currentNodeIndex = nodesToVisit[toVisitOffset - 1];
			toVisitOffset--;
#endif
		}
	}
}