#include "world.hpp"

RTVE::SparseVoxelOctree::SparseVoxelOctree(uint pSize, uint pMaxDepth)
:mSize(pSize), mMaxDepth(pMaxDepth) {
  // mNodes.push_back(Node());
}

void RTVE::SparseVoxelOctree::insert(const glm::vec3& pPoint, const VoxelData& pData) {
  uint i = 1;
  insertImpl(&i, pPoint, pData, glm::tvec3<int>(0, 0, 0), 0);
}

uint RTVE::SparseVoxelOctree::getSize() {
  return mSize;
}

uint RTVE::SparseVoxelOctree::getMidpoint() {
  return 10;
}

void RTVE::SparseVoxelOctree::insertImpl(uint* pNodeIndex, const glm::vec3& pPoint, const VoxelData& pData, glm::tvec3<int> pPosition, uint pDepth) {
  // // If node doesn't exist create it
  // if (*pNodeIndex == 0) {
  //   *pNodeIndex = mNodes.size();
  //   mNodes.push_back(Node());
  // }

  // Node& node = mNodes.at((*pNodeIndex)-1); // Minus one from node index because 0 is used as null

  // node.data = pData;
  // if (pDepth == mMaxDepth) {
  //   // Return once we reach max depth
  //   node.isLeaf = true;
  //   return;
  // }

  // // float size = mSize / std::exp2(pDepth); // Size at the current resolution = Total size / (2 to the power of the current depth)
  // float size = ldexpf(mSize, pDepth);

  // glm::tvec3<int> childPos = {
  //   pPoint.x >= (size * pPosition.x) + (size / 2.f),
  //   pPoint.y >= (size * pPosition.y) + (size / 2.f),
  //   pPoint.z >= (size * pPosition.z) + (size / 2.f)
  // };

  // uint childIndex = (childPos.x << 0) | (childPos.y << 1) | (childPos.z << 2);

  // pPosition = {
  //   (pPosition.x << 1) | childPos.x, // Position is pased down by putting it in front of the childPos
  //   (pPosition.y << 1) | childPos.y,
  //   (pPosition.z << 1) | childPos.z
  // };
  
  // insertImpl(&node.childrenIndices[childIndex], pPoint, pData, pPosition, ++pDepth);
}

