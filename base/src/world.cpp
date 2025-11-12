#include "world.hpp"

RTVE::SparseVoxelDAG::SparseVoxelDAG(uint pSize)
:mSize(pSize), mMaxDepth(log2(pSize)), mMidpoint(UINT_MAX / 2) {
  mIndices.push_back({mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint});
}

void RTVE::SparseVoxelDAG::insert(const glm::vec3& pPoint, const VoxelData& pData) {
  const auto result = std::find(std::begin(mData), std::end(mData), pData);
  uint index;
  if (result != mData.end())
    index = std::distance(mData.begin(), result);
  else {
    index = mData.size();
    mData.push_back(pData);
  }
  insertImpl(pPoint, index, 0, mSize, glm::vec3(0, 0, 0));
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

void RTVE::SparseVoxelDAG::drawDebug() {
  glBindVertexArray(mDebugVAO);
  // glDrawElements(GL_LINE_STRIP, count, GL_UNSIGNED_INT, 0);
  glDrawArrays(GL_LINE_STRIP, 0, 36);
  glBindVertexArray(0);
}

uint RTVE::SparseVoxelDAG::toChildIndex(glm::vec3 pPos) {
  glm::tvec3<int, glm::packed_highp> localChildPos = {
    int(floor(pPos.x)),
    int(floor(pPos.y)),
    int(floor(pPos.z))
  };
  return (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7
}

void RTVE::SparseVoxelDAG::generateDebugMesh() {
  // Generate mesh
  // std::vector<glm::vec3> vertcies;
  // std::vector<uint> indices;

  float vertices[] = {
    // back face
    -1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    // front face
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    // left face
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    // right face
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    // bottom face
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    // top face
    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f , 1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f
  };
  
  glGenVertexArrays(1, &mDebugVAO);
  glGenBuffers(1, &mDebugVBO);
  // glGenBuffers(1, &mDebugEBO);

  glBindVertexArray(mDebugVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mDebugVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mDebugEBO);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(uint), &indices[0], GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void RTVE::SparseVoxelDAG::releaseDebugMesh() {
  glDeleteVertexArrays(1, &mDebugVAO);
  glDeleteBuffers(1, &mDebugVBO);
  glDeleteBuffers(1, &mDebugEBO);
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

