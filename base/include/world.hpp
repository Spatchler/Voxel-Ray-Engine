#pragma once

#include <vector>
#include <print>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RTVE {
  struct VoxelData {
    glm::vec4 color;
  };

  class SparseVoxelDAG {
  public:
    SparseVoxelDAG(uint pSize);

    void insert(const glm::vec3& pPoint, const VoxelData& pData);

    uint getSize();
    uint getMidpoint();

    void print();

    static uint toChildIndex(glm::vec3 pPos);

    std::vector<std::array<uint, 8>> mIndices;
    std::vector<VoxelData> mData;
  private:
    void insertImpl(const glm::vec3& pPoint, const uint& pData, uint pNodeIndex, uint pNodeSize, glm::vec3 pNodeOrigin);

    uint mSize, mMaxDepth, mMidpoint;
  };
}

