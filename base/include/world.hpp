#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RTVE {
  struct VoxelData {
    glm::vec4 color;
  };

  class SparseVoxelOctree {
  public:
    SparseVoxelOctree(uint pSize, uint pMaxDepth);

    void insert(const glm::vec3& pPoint, const VoxelData& pData);

    uint getSize();
    uint getMidpoint();

    std::vector<std::array<uint, 8>> mIndices;
    std::vector<VoxelData> mData;
  private:
    void insertImpl(uint* pNodeIndex, const glm::vec3& pPoint, const VoxelData& pData, glm::tvec3<int> pPosition, uint pDepth);

    uint mSize;
    uint mMaxDepth;
  };
}

