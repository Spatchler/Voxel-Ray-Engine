#pragma once

#include <vector>
#include <print>
#include <algorithm>

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RTVE {
  struct VoxelData {
    glm::vec4 color;
  };

  constexpr bool operator==(const VoxelData& lhs, const VoxelData& rhs) {
    return lhs.color == rhs.color;
  }

  class SparseVoxelDAG {
  public:
    SparseVoxelDAG(uint pSize);

    void insert(const glm::vec3& pPoint, const VoxelData& pData);

    uint getSize();
    uint getMidpoint();

    void print();

    void drawDebug();

    static uint toChildIndex(glm::vec3 pPos);

    void generateDebugMesh();
    void releaseDebugMesh();

    std::vector<std::array<uint, 8>> mIndices;
    std::vector<VoxelData> mData;
  private:
    void insertImpl(const glm::vec3& pPoint, const uint& pData, uint pNodeIndex, uint pNodeSize, glm::vec3 pNodeOrigin);

    uint mDebugVAO, mDebugVBO, mDebugEBO;
    uint mSize, mMaxDepth, mMidpoint;
  };
}

