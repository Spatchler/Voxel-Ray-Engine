#pragma once

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace RTVE {
  struct VoxelData {
    glm::vec4 color;
  };

  struct Node {
    bool isLeaf = false;
    uint childrenIndices[8] = {0,0,0,0,0,0,0,0};
    VoxelData data;
  };

  class SparseVoxelOctree {
  public:
    SparseVoxelOctree(uint pSize, uint pMaxDepth);

    void insert(const glm::vec3& pPoint, const VoxelData& pData);

    std::vector<Node> mNodes;
  private:
    void insertImpl(uint* pNodeIndex, const glm::vec3& pPoint, const VoxelData& pData, glm::tvec3<int> pPosition, uint pDepth);

    uint mSize;
    uint mMaxDepth;
  };
}

