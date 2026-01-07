#pragma once

#include "timer.hpp"
#include "shader.hpp"

// #include "VMesh/voxelGrid.hpp"

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
    SparseVoxelDAG(const std::string& pPath);
    SparseVoxelDAG(const std::vector<std::vector<std::vector<bool>>>& pGrid);

    void insert(const glm::vec3& pPoint, const VoxelData& pData);

    constexpr uint getSize() { return mSize; }
    constexpr uint getMidpoint() { return mMidpoint; }
    constexpr uint getMaxDepth() { return mMaxDepth; }

    void print();

    virtual void drawDebug(Shader* pShader);

    static uint toChildIndex(glm::vec3 pPos);
    static glm::vec3 toPos(uint pChildIndex);

    void generateDebugMesh();
    void releaseDebugMesh();

    std::vector<std::array<uint32_t, 8>> mIndices;
    std::vector<VoxelData> mData;
  protected:
    void loadFromFile(const std::string& pPath);

    uint generateSVDAGTopDown(const std::vector<std::vector<std::vector<bool>>>& pGrid, glm::vec3 pNodeOrigin, uint pNodeSize, std::vector<std::tuple<uint, uint, glm::vec3, uint>>& pQueue);

    void insertImpl(const glm::vec3& pPoint, const uint& pData, uint pNodeIndex, uint pNodeSize, glm::vec3 pNodeOrigin);

    void insertCubeVerticesImpl(std::vector<glm::vec3>& pVertices, uint pNodeSize, glm::vec3 pNodeOrigin, uint pNodeIndex);

    void insertCubeVertices(std::vector<glm::vec3>& pVertices, glm::vec3 pPos, float pScale) const;

    uint mDebugVertexCount;

    uint mSize, mMaxDepth, mMidpoint;
#ifdef _DEBUG
    uint mDebugVAO, mDebugVBO;
#endif
  };
}

