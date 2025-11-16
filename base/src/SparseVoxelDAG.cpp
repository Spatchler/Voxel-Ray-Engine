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
  glDrawArrays(GL_LINES, 0, mDebugVertexCount);
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

glm::vec3 RTVE::SparseVoxelDAG::toPos(uint pChildIndex) {
  glm::vec3 pos;
  pos.x = 1 & pChildIndex;
  pos.y = ((1 << 1) & pChildIndex) != 0;
  pos.z = ((1 << 2) & pChildIndex) != 0;
  return pos;
}

void RTVE::SparseVoxelDAG::generateDebugMesh() {
  // Generate mesh
  std::vector<glm::vec3> vertices;
  // std::vector<uint> indices;

  insertCubeVerticesImpl(vertices, mSize, glm::vec3(0, 0, 0), 0);

  mDebugVertexCount = vertices.size();
  std::println("Count: {}", mDebugVertexCount);
  
  glGenVertexArrays(1, &mDebugVAO);
  glGenBuffers(1, &mDebugVBO);
  // glGenBuffers(1, &mDebugEBO);

  glBindVertexArray(mDebugVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mDebugVBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

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

void RTVE::SparseVoxelDAG::insertCubeVerticesImpl(std::vector<glm::vec3>& pVertices, uint pNodeSize, glm::vec3 pNodeOrigin, uint pNodeIndex) {
  insertCubeVertices(pVertices, pNodeOrigin, pNodeSize);
  if (pNodeIndex == mMidpoint) {
    return;
  }
  for (uint i = 0; i < 8; ++i) {
    uint index = mIndices[pNodeIndex][i];
    if (index <= mMidpoint)
      insertCubeVerticesImpl(pVertices, pNodeSize >> 1, pNodeOrigin + toPos(i) * (float)(pNodeSize >> 1), index);
  }
}

void RTVE::SparseVoxelDAG::insertCubeVertices(std::vector<glm::vec3>& pVertices, glm::vec3 pPos, float pScale) const {
  float vertices[] = {
    0.f, 0.f, 0.f,
    1.f, 0.f, 0.f,

    1.f, 0.f, 0.f,
    1.f, 1.f, 0.f,

    1.f, 1.f, 0.f,
    0.f, 1.f, 0.f,

    0.f, 1.f, 0.f,
    0.f, 0.f, 0.f,

    0.f, 0.f, 0.f,
    0.f, 0.f, 1.f,

    0.f, 1.f, 0.f,
    0.f, 1.f, 1.f,

    1.f, 1.f, 0.f,
    1.f, 1.f, 1.f,

    1.f, 0.f, 0.f,
    1.f, 0.f, 1.f,

    0.f, 0.f, 1.f,
    0.f, 1.f, 1.f,

    0.f, 1.f, 1.f,
    1.f, 1.f, 1.f,

    1.f, 1.f, 1.f,
    1.f, 0.f, 1.f,

    1.f, 0.f, 1.f,
    0.f, 0.f, 1.f
  };

  for (uint i = 0; i < sizeof(vertices) / (3*sizeof(float)); ++i) {
    pVertices.push_back(glm::vec3(vertices[i*3], vertices[i*3+1], vertices[i*3+2]) * (pScale / mSize) + (pPos / (float)mSize));
  }
}

