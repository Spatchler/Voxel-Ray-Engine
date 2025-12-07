#include "model.hpp"

RTVE::SVDAGModel::SVDAGModel(const std::string& pPath, uint pResolution)
:SparseVoxelDAG(pResolution), mResolution(pResolution) {
  // Init mVoxelGrid
  mVoxelGrid = std::vector<std::vector<std::vector<bool>>>(pResolution, std::vector<std::vector<bool>>(pResolution, std::vector<bool>(pResolution, false)));

  loadModel(pPath);
}

void RTVE::SVDAGModel::drawDebug(Shader* pShader) {
#ifdef _DEBUG
  // Tree
  pShader->setVec3("uColour", glm::vec3(1, 0, 0));

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::scale(model, glm::vec3(mSize, mSize, mSize));
  pShader->setMat4("uModel", model);

  glBindVertexArray(mDebugVAO);
  glDrawArrays(GL_LINES, 0, mDebugVertexCount);
  glBindVertexArray(0);

  // Model
  pShader->setVec3("uColour", glm::vec3(0, 1, 0));

  model = glm::mat4(1.0f);
  pShader->setMat4("uModel", model);

  glBindVertexArray(mModelDebugVAO);
  glDrawElements(GL_LINES, mDebugLineIndicesCount, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
#endif
}

void RTVE::SVDAGModel::generateModelDebugMesh() {
#ifdef _DEBUG
  mDebugLineIndices.clear();

  generateDebugLineIndices();

  mDebugLineIndicesCount = mDebugLineIndices.size();

  glGenVertexArrays(1, &mModelDebugVAO);
  glGenBuffers(1, &mModelDebugVBO);
  glGenBuffers(1, &mModelDebugEBO);

  glBindVertexArray(mModelDebugVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mModelDebugVBO);
  glBufferData(GL_ARRAY_BUFFER, mMeshVertices.size() * sizeof(glm::vec3), &mMeshVertices[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mModelDebugEBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mDebugLineIndices.size() * sizeof(uint), &mDebugLineIndices[0], GL_STATIC_DRAW);

  glBindVertexArray(0);

  mDebugLineIndices.clear();
#endif
}

void RTVE::SVDAGModel::releaseModelDebugMesh() {
#ifdef _DEBUG
  glDeleteVertexArrays(1, &mModelDebugVAO);
  glDeleteBuffers(1, &mModelDebugVBO);
  glDeleteBuffers(1, &mModelDebugEBO);
#endif
}

void RTVE::SVDAGModel::loadModel(const std::string& pPath) {
  RTVE::ScopedTimer loadModelTimer(std::format("loadModel: {}", pPath));

  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(pPath, aiProcess_Triangulate);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::println("ERROR::ASSIMP::{0}", importer.GetErrorString());
    return;
  }

  processNode(scene->mRootNode, scene);

  // mMeshVertices.push_back(glm::vec3(10, 10, 10));
  // mMeshVertices.push_back(glm::vec3(20, 20, 10));
  // mMeshVertices.push_back(glm::vec3(10, 20, 20));
  // mMeshVertices.push_back(glm::vec3(20, 30, 20));

  // mMeshVertices.push_back(glm::vec3(10, 10, 10));
  // mMeshVertices.push_back(glm::vec3(20, 10, 10));
  // mMeshVertices.push_back(glm::vec3(10, 10, 20));
  // mMeshVertices.push_back(glm::vec3(21, 21, 21));

  // mMeshIndices.push_back(0);
  // mMeshIndices.push_back(1);
  // mMeshIndices.push_back(2);
  
  // mMeshIndices.push_back(3);
  // mMeshIndices.push_back(1);
  // mMeshIndices.push_back(2);

  // mMeshVertices.push_back(glm::vec3(30, 30, 30));
  // mMeshVertices.push_back(glm::vec3(52, 30, 30));
  // mMeshVertices.push_back(glm::vec3(30, 35, 29));
  // mMeshVertices.push_back(glm::vec3(29, 35, 29));
  // mMeshVertices.push_back(glm::vec3(29, 35, 30));
  // mMeshIndices.push_back(0);
  // mMeshIndices.push_back(1);
  // mMeshIndices.push_back(2);

  mData.push_back({glm::vec4(0.1, 0.1, 0.1, 0)});

  mVoxelCount = 0u;
  mTriangleCount = mMeshIndices.size() / 3u;

  float triangleCountInv = 1.f/mTriangleCount;
  uint countSinceLastPrint = 0u;
  // uint progressDelay = std::max(1u, mTriangleCount / 100u);
  uint progressDelay = mTriangleCount / 100u;
  std::println("ProgressDelay: {}", progressDelay);
  std::println("ProgressDelay: {}", progressDelay);

  for (uint i = 0; i < mMeshVertices.size(); ++i) {
    // Green player
    // mMeshVertices[i] *= mResolution * 2;
    // mMeshVertices[i] += mResolution * 0.5f;

    // Sponza
    mMeshVertices[i] *= mResolution * 0.0234375f;
    mMeshVertices[i] += mResolution * 0.5f;
  }

  for (uint i = 0; i < mTriangleCount; ++i) { // Every triangle
    std::println("Tri: {}", i);
    // Add points to array
    std::array<glm::vec3, 3> points;
    for (uint j = 0; j < 3; ++j) {
      // glm::vec3 point = mMeshVertices[mMeshIndices[i*3+j]];
      // point *= 30.f; // Tyra
      // point *= 22.f; // Hollow Knight
      // point.x += mResolution / 2.f;
      // point.y += mResolution / 2.f;
      // point.z += mResolution / 2.f;

      // Bunnny
      // point *= 50.f;
      // point.x += mResolution / 1.8f;
      // point.y += mResolution * 0.1f;
      // point.z += mResolution * 0.5f;
      
      // Dragon
      // point += 1;
      // point *= 100;
      // point += mResolution * 0.5f;

      // points[j] = point;
      points[j] = mMeshVertices[mMeshIndices[i*3+j]];
    }

    // Find dominant axis
    glm::vec3 minPoint = glm::min(points[0], glm::min(points[1], points[2]));
    glm::vec3 maxPoint = glm::max(points[0], glm::max(points[1], points[2]));
    glm::vec3 boxSize = maxPoint - minPoint;

    uint8_t dominantAxisIndex = 0;
    if ((boxSize.x >= boxSize.y) && (boxSize.x >= boxSize.z))
      dominantAxisIndex = 0;
    else if ((boxSize.y >= boxSize.x) && (boxSize.y >= boxSize.z))
      dominantAxisIndex = 1;
    else if ((boxSize.z >= boxSize.x) && (boxSize.z >= boxSize.y))
      dominantAxisIndex = 2;
    std::array<uint8_t, 2> nonDominantAxisIndices;
    nonDominantAxisIndices[0] = (dominantAxisIndex + 1) % 3;
    nonDominantAxisIndices[1] = (dominantAxisIndex + 2) % 3;

    // glm::vec3 normal = glm::cross(points[1] - points[0], points[2] - points[0]);
    // normal = glm::normalize(normal);

    // Sort by dominant axis
    if (v3index(points[0], dominantAxisIndex) > v3index(points[1], dominantAxisIndex)) std::swap(points[0], points[1]);
    if (v3index(points[1], dominantAxisIndex) > v3index(points[2], dominantAxisIndex)) std::swap(points[1], points[2]);
    if (v3index(points[0], dominantAxisIndex) > v3index(points[1], dominantAxisIndex)) std::swap(points[0], points[1]);

    // Calculate direction and inverse for lines
    glm::vec3 lhDir = points[2] - points[0];
    glm::vec3 lhDirInv = 1.f/lhDir;

    glm::vec3 lmDir = points[1] - points[0];
    glm::vec3 lmDirInv = 1.f/lmDir;

    glm::vec3 mhDir = points[2] - points[1];
    glm::vec3 mhDirInv = 1.f/mhDir;

    float blueValue = 0.f;
    // Iterate over along the dominant axis traversing between the two lines
    for (float dominantAxisValue = v3index(points[0], dominantAxisIndex); dominantAxisValue <= v3index(points[2], dominantAxisIndex); ++dominantAxisValue) {
      glm::vec3* l1Dir = &lmDir;
      glm::vec3* l1DirInv = &lmDirInv;
      glm::vec3* l2Dir = &lhDir;
      glm::vec3* l2DirInv = &lhDirInv;
      if ((v3index(points[1], dominantAxisIndex) < dominantAxisValue) || (v3index(lmDir, dominantAxisIndex) == 0)) {
        l1Dir = &mhDir;
        l1DirInv = &mhDirInv;
        l2Dir = &lhDir;
        l2DirInv = &lhDirInv;
      }

      glm::vec2 l1Pos;
      glm::vec2 l2Pos;

      float l2t = (dominantAxisValue - v3index(points[0], dominantAxisIndex)) * v3index(*l2DirInv, dominantAxisIndex);
      l2Pos = glm::vec2(v3index(points[0], nonDominantAxisIndices[0]), v3index(points[0], nonDominantAxisIndices[1])) // Origin
                        + l2t * glm::vec2(v3index(*l2Dir, nonDominantAxisIndices[0]), v3index(*l2Dir, nonDominantAxisIndices[1]));

      if ((v3index(points[1], dominantAxisIndex) >= dominantAxisValue) && (v3index(points[1], dominantAxisIndex) <= dominantAxisValue + 1)) {
        l1Pos = glm::vec2(v3index(points[1], nonDominantAxisIndices[0]), v3index(points[1], nonDominantAxisIndices[1]));
        std::println("Hi---------------------------------------------------------------------------");
      }
      else {
        std::println("Doing double method --------------------------------------------------------------------------- ");
        float l1t = (dominantAxisValue - v3index(points[1], dominantAxisIndex)) * v3index(*l1DirInv, dominantAxisIndex);
        glm::vec2 l1Posa = glm::vec2(v3index(points[1], nonDominantAxisIndices[0]), v3index(points[1], nonDominantAxisIndices[1])) // Origin
                          + l1t * glm::vec2(v3index(*l1Dir, nonDominantAxisIndices[0]), v3index(*l1Dir, nonDominantAxisIndices[1]));
        l1t = (std::min(dominantAxisValue + 1, v3index(points[2], dominantAxisIndex)) - v3index(points[1], dominantAxisIndex)) * v3index(*l1DirInv, dominantAxisIndex);
        glm::vec2 l1Posb = glm::vec2(v3index(points[1], nonDominantAxisIndices[0]), v3index(points[1], nonDominantAxisIndices[1])) // Origin
                          + l1t * glm::vec2(v3index(*l1Dir, nonDominantAxisIndices[0]), v3index(*l1Dir, nonDominantAxisIndices[1]));
        if (glm::length(l2Pos - l1Posa) >= glm::length(l2Pos - l1Posb))
          l1Pos = l1Posa;
        else
          l1Pos = l1Posb;
      }

      glm::vec2 dir = l2Pos - l1Pos; // Dir from l1 to l2 intersection points
      glm::vec2 dirInv = 1.f/dir;
      // if (dir.x != 0)
        // dir.x /= glm::length(dir);
      // if (dir.y != 0)
        // dir.y /= glm::length(dir);

      // glm::vec2 voxelPos = glm::floor(l1Pos) - glm::sign(dir);
      // glm::vec2 voxelPos = glm::floor(l1Pos) - glm::sign(glm::vec2(std::max(dir.x, dir.y) == dir.x, std::max(dir.x, dir.y) == dir.y));
      // glm::vec2 voxelPos = glm::floor(l1Pos - dir);

      // std::println("Dir: {}, {}, voxelPos: {}, {}, dirInvSigns: {}, {}, dirSigns: {}, {}", dir.x, dir.y, voxelPos.x, voxelPos.y, glm::sign(dirInv.x), glm::sign(dirInv.y), glm::sign(dir.x), glm::sign(dir.y));

      std::println("l1Pos: {}, {}, l2Pos: {}, {}", l1Pos.x, l1Pos.y, l2Pos.x, l2Pos.y);

      // Ray start pos is going wrong
      drawLine2(dominantAxisIndex, dominantAxisValue, l1Pos, l2Pos, dir, dirInv, blueValue);
      blueValue += 0.1f;
      if (blueValue > 1)
        blueValue = 0.f;
    }

    if (mTriangleCount > 100) {
      ++countSinceLastPrint;
      if (countSinceLastPrint > progressDelay) {
        printProgressBar((i + 1) * triangleCountInv, "Voxelizing:");
        countSinceLastPrint = 0;
      }
    }
  }

#ifndef _DEBUG
  mMeshVertices.clear();
  mMeshIndices.clear();
  mVoxelGrid.clear();
#endif
}

void RTVE::SVDAGModel::processNode(aiNode* pNode, const aiScene* pScene) {
  for (uint i = 0; i < pNode->mNumMeshes; ++i) {
    aiMesh* mesh = pScene->mMeshes[pNode->mMeshes[i]];
    processMesh(mesh, pScene);
  }

  for (uint i = 0; i < pNode->mNumChildren; ++i)
    processNode(pNode->mChildren[i], pScene);
}

void RTVE::SVDAGModel::processMesh(aiMesh* pMesh, const aiScene* pScene) {
  uint verticesSize = mMeshVertices.size();

  for (uint i = 0; i < pMesh->mNumVertices; ++i) {
    glm::vec3 pos;

    pos.x = pMesh->mVertices[i].x;
    pos.y = pMesh->mVertices[i].y;
    pos.z = pMesh->mVertices[i].z;

    mMeshVertices.push_back(pos);
  }

  for (uint i = 0; i < pMesh->mNumFaces; ++i) {
    aiFace face = pMesh->mFaces[i];
    for (uint j = 0; j < face.mNumIndices; ++j)
      mMeshIndices.push_back(face.mIndices[j] + verticesSize);
  }
}

void RTVE::SVDAGModel::drawLine2(uint8_t pDominantAxisIndex, float pDominantAxisValue, const glm::vec2& pStart, const glm::vec2& pEnd, const glm::vec2& pDir, const glm::vec2& pDirInv, float pBlueValue) {
  float redValue = 0.f;
  glm::vec3 v = glm::floor(toVec3(pDominantAxisValue, pStart.x, pStart.y, pDominantAxisIndex));
  if (!mVoxelGrid[v.x][v.y][v.z]) {
    ++mVoxelCount;
    insert(v, {glm::vec4(1, 1, 1, 0)});
    mVoxelGrid[v.x][v.y][v.z] = true;
  }

  glm::vec2 voxelPos = glm::floor(pStart);

  while (voxelPos != glm::floor(pEnd)) {
    std::println("voxelPos: {}, {}, l2pos: {}, {}, signDirInv: {}, {}", voxelPos.x, voxelPos.y, pEnd.x, pEnd.y, glm::sign(pDirInv.x), glm::sign(pDirInv.y));
  // while ( (((voxelPos.x <= pEnd.x) * std::max(0.f, glm::sign(pDirInv.x))) ||
  //          ((voxelPos.x >= pEnd.x) * std::max(0.f, glm::sign(pDirInv.x) * -1))) &&
  //         (((voxelPos.y <= pEnd.y) * std::max(0.f, glm::sign(pDirInv.y))) ||
  //          ((voxelPos.y >= pEnd.y) * std::max(0.f, glm::sign(pDirInv.y) * -1))) ) {
    float plane1 = voxelPos.x + std::max(0.f, glm::sign(pDirInv.x));
    float plane2 = voxelPos.y + std::max(0.f, glm::sign(pDirInv.y));

    // if (plane1 == pEnd.x && plane2 == pEnd.y) { // If the destination point lies exactly on the planes
    //   voxelPos += glm::sign(pDirInv);
    //   v = glm::floor(toVec3(pDominantAxisValue, voxelPos.x, voxelPos.y, pDominantAxisIndex));
    //   if (!mVoxelGrid[v.x][v.y][v.z]) {
    //     ++mVoxelCount;
    //     insert(v, {glm::vec4(redValue, 0, 0, 0)});
    //     mVoxelGrid[v.x][v.y][v.z] = true;
    //   }
    //   break;
    // }
    
    float t1 = (plane1 - pStart.x) * pDirInv.x;
    float t2 = (plane2 - pStart.y) * pDirInv.y;

    if (t1 < t2)
      voxelPos.x += glm::sign(pDirInv.x);
    else
      voxelPos.y += glm::sign(pDirInv.y);

    v = glm::floor(toVec3(pDominantAxisValue, voxelPos.x, voxelPos.y, pDominantAxisIndex));
    if (!mVoxelGrid[v.x][v.y][v.z]) {
      ++mVoxelCount;
      // insert(v, {glm::vec4(redValue, 0, 0, 0)});
      insert(v, {glm::vec4(1, 1, 1, 0)});
      redValue += 0.3f;
      if (redValue > 1.f)
        redValue = 0.f;
      mVoxelGrid[v.x][v.y][v.z] = true;
    }
  }

  // std::println("Started");
  // int steps;
  // steps = std::max(abs(pDir.x), abs(pDir.y));
  // std::println("Steps: {}, dir: {}, {}, ceil(dir): {}, {}", steps, pDir.x, pDir.y, abs(pDir.x), abs(pDir.y));
  // glm::vec2 incr = pDir / (float)steps;
  // glm::vec2 voxelPos = pStart;
  // for (uint i = 0; i < steps; ++i) {
  //   glm::vec3 v = toVec3(pDominantAxisValue, glm::floor(voxelPos.x), glm::floor(voxelPos.y), pDominantAxisIndex);
  //   // glm::vec3 v = toVec3(a, floor(voxelPos.x), floor(voxelPos.y), dominantAxisIndex);
  //   if (!mVoxelGrid[v.x][v.y][v.z]) {
  //     std::println("Inserting");
  //     insert(v, {glm::vec4(redValue, 0, pBlueValue, 0)});
  //     redValue += 0.1f;
  //     if (redValue > 1.f)
  //       redValue = 0.f;
  //     mVoxelGrid[v.x][v.y][v.z] = true;
  //   }
  //   voxelPos += incr;
  // }
  // std::println("pEnd: {}, {}", pEnd.x, pEnd.y);
  // glm::vec3 v = glm::floor(toVec3(pDominantAxisValue, pEnd.x, pEnd.y, pDominantAxisIndex));
  // if (!mVoxelGrid[v.x][v.y][v.z]) {
  //   ++mVoxelCount;
  //   insert(v, {glm::vec4(0, 0, 1, 0)});
  //   mVoxelGrid[v.x][v.y][v.z] = true;
  // }
  // std::println("Finished");
}

glm::vec3 RTVE::SVDAGModel::toVec3(float a, float b, float c, uint8_t pDominantAxisIndex) {
  glm::vec3 v;
  if (pDominantAxisIndex == 0) {
    v.x = a;
    v.y = b;
    v.z = c;
  }
  else if (pDominantAxisIndex == 1) {
    v.x = c;
    v.y = a;
    v.z = b;
  }
  else if (pDominantAxisIndex == 2) {
    v.x = b;
    v.y = c;
    v.z = a;
  }
  return v;
}

void RTVE::SVDAGModel::generateDebugLineIndices() {
  for (uint i = 0; i < mTriangleCount; ++i) {
    mDebugLineIndices.push_back(mMeshIndices[i * 3]);
    mDebugLineIndices.push_back(mMeshIndices[i * 3 + 1]);
    mDebugLineIndices.push_back(mMeshIndices[i * 3 + 1]);
    mDebugLineIndices.push_back(mMeshIndices[i * 3 + 2]);
    mDebugLineIndices.push_back(mMeshIndices[i * 3 + 2]);
    mDebugLineIndices.push_back(mMeshIndices[i * 3]);
  }
}

