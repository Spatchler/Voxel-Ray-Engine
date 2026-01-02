#include "SparseVoxelDAG.hpp"

RTVE::SparseVoxelDAG::SparseVoxelDAG(uint pSize)
:mSize(pSize), mMaxDepth(log2(pSize)), mMidpoint(UINT_MAX / 2) {
  mIndices.push_back({mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint, mMidpoint});
}

RTVE::SparseVoxelDAG::SparseVoxelDAG(const std::string& pPath)
:mMidpoint(UINT_MAX / 2) {
  loadFromFile(pPath);
}

void RTVE::SparseVoxelDAG::insert(const glm::vec3& pPoint, const VoxelData& pData) {
  std::println("Inserting: {}, {}, {}", pPoint.x, pPoint.y, pPoint.z);
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
  std::println("Size: {}", mSize);
  std::println("------------------------------------\n");
}

void RTVE::SparseVoxelDAG::drawDebug(Shader* pShader) {
#ifdef _DEBUG
  pShader->setVec3("uColour", glm::vec3(1, 0, 0));

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::scale(model, glm::vec3(mSize, mSize, mSize));
  pShader->setMat4("uModel", model);

  glBindVertexArray(mDebugVAO);
  glDrawArrays(GL_LINES, 0, mDebugVertexCount);
  glBindVertexArray(0);
#endif
}

uint RTVE::SparseVoxelDAG::toChildIndex(glm::vec3 pPos) {
  glm::tvec3<int, glm::packed_highp> localChildPos = {
    int(std::min(1.0, floor(pPos.x))),
    int(std::min(1.0, floor(pPos.y))),
    int(std::min(1.0, floor(pPos.z)))
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
#ifdef _DEBUG
  RTVE::ScopedTimer t("Generating debug mesh");

  // Generate mesh
  std::vector<glm::vec3> vertices;
  // std::vector<uint> indices;

  insertCubeVerticesImpl(vertices, mSize, glm::vec3(0, 0, 0), 0);

  mDebugVertexCount = vertices.size();
  
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
#endif
}

void RTVE::SparseVoxelDAG::releaseDebugMesh() {
#ifdef _DEBUG
  glDeleteVertexArrays(1, &mDebugVAO);
  glDeleteBuffers(1, &mDebugVBO);
#endif
}

void RTVE::SparseVoxelDAG::loadFromFile(const std::string& pPath) {
  ScopedTimer t("Loading file");
  std::ifstream fin;
  fin.open(pPath, std::ios::binary | std::ios::in);

  // Read resolution
  uint resolution;
  fin.read(reinterpret_cast<char*>(&resolution), 4);
  mSize = resolution;
  mMaxDepth = std::log2(mSize);

  // Load indices
  mIndices.clear();
  uint32_t value;
  uint i = 0;
  std::array<uint32_t, 8> node;
  while (fin.read(reinterpret_cast<char*>(&value), sizeof(uint32_t))) {
    if (value == 0)
      node[i] = mMidpoint;
    else if (value == 1)
      node[i] = mMidpoint + 1;
    else
      node[i] = value - 2;
    ++i;
    if (i == 8) {
      mIndices.push_back(node);
      i = 0;
    }
  }
}

void RTVE::SparseVoxelDAG::insertImpl(const glm::vec3& pPoint, const uint& pDataIndex, uint pNodeIndex, uint pNodeSize, glm::vec3 pNodeOrigin) {
  // TODO: Add dynamic midpoint
  pNodeSize = pNodeSize >> 1;

  glm::vec3 pos = pPoint;
  pos -= pNodeOrigin;
  pos /= pNodeSize;
  pos = glm::floor(pos);
  pNodeOrigin += pos * (float)pNodeSize;
  uint& node = mIndices[pNodeIndex][toChildIndex(pos)];
  pNodeIndex = node;

  if (pNodeSize == 1) { // Insert data index once we reach max depth
    node = mMidpoint + pDataIndex;
    return;
  }

  if (node > mMidpoint)
    return;

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

