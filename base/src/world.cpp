#include "world.hpp"

RTVE::SparseVoxelDAG::SparseVoxelDAG(uint pSize)
:mSize(pSize), mMaxDepth(log2(pSize)), mMidpoint(20) {
  mIndices.push_back({mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint});
}

void RTVE::SparseVoxelDAG::insert(const glm::vec3& pPoint, const VoxelData& pData) {
  mData.push_back(pData);
  insertImpl(pPoint, mData.size() - 1, 0, mSize, glm::vec3(0, 0, 0));
}

uint RTVE::SparseVoxelDAG::getSize() {
  return mSize;
}

uint RTVE::SparseVoxelDAG::getMidpoint() {
  return mMidpoint;
}

void RTVE::SparseVoxelDAG::print() {
  std::println("\n------------------------------------");
  std::println("Indices:");
  for (auto node: mIndices) {
    std::println("{}, {}, {}, {}, {}, {}, {}, {}", node[0], node[1], node[2], node[3], node[4], node[5], node[6], node[7]);
  }
  std::println("Data:");
  for (VoxelData& node: mData) {
    std::println("{}, {}, {}, {}", node.color.x, node.color.y, node.color.z, node.color.w);
  }
  std::println("------------------------------------\n");
}

uint RTVE::SparseVoxelDAG::toChildIndex(glm::vec3 pPos) {
  glm::tvec3<int, glm::packed_highp> localChildPos = {
    int(floor(pPos.x)),
    int(floor(pPos.y)),
    int(floor(pPos.z))
  };
  return (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7
}

void RTVE::SparseVoxelDAG::insertImpl(const glm::vec3& pPoint, const uint& pDataIndex, uint pNodeIndex, uint pNodeSize, glm::vec3 pNodeOrigin) {
  // TODO: Add dynamic midpoint
  pNodeSize = pNodeSize >> 1;

  glm::vec3 pos = pPoint;
  pos -= pNodeOrigin;
  pos /= pNodeSize;
  pos.x = floor(pos.x);
  pos.y = floor(pos.y);
  pos.z = floor(pos.z);
  pNodeOrigin += pos * (float)pNodeSize;
  uint& node = mIndices.at(pNodeIndex)[toChildIndex(pos)];
  pNodeIndex = node;

  if (pNodeSize == 1) { // Insert data index once we reach max depth
    node = mMidpoint + pDataIndex;
    return;
  }

  // If node doesn't exist create it
  if (node == 0 || node == mMidpoint) {
    pNodeIndex = mIndices.size();
    node = mIndices.size();
    mIndices.push_back({mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint}); // Air node
  }

  insertImpl(pPoint, pDataIndex, pNodeIndex, pNodeSize, pNodeOrigin);
}

