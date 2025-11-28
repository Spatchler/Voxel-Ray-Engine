#include "model.hpp"

RTVE::SVDAGModel::SVDAGModel(const std::string& pPath, uint pResolution)
:SparseVoxelDAG(pResolution), mResolution(pResolution) {
  // Init mVoxelGrid
  mVoxelGrid = std::vector<std::vector<std::vector<bool>>>(pResolution, std::vector<std::vector<bool>>(pResolution, std::vector<bool>(pResolution, false)));

  loadModel(pPath);
}

void RTVE::SVDAGModel::loadModel(const std::string& pPath) {
  RTVE::ScopedTimer loadModelTimer(std::format("loadModel: {}", pPath));

  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(pPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::println("ERROR::ASSIMP::{0}", importer.GetErrorString());
    return;
  }

  mDirectory = pPath.substr(0, pPath.find_last_of('/'));

  processNode(scene->mRootNode, scene);

  // mMeshVertices.push_back(glm::vec3(10, 10, 10));
  // mMeshVertices.push_back(glm::vec3(20, 10, 10));
  // mMeshVertices.push_back(glm::vec3(10, 10, 20));
  // mMeshVertices.push_back(glm::vec3(21, 10, 21));

  // mMeshIndices.push_back(0);
  // mMeshIndices.push_back(1);
  // mMeshIndices.push_back(2);
  
  // mMeshIndices.push_back(3);
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

  for (uint i = 0; i < mTriangleCount; ++i) { // Every triangle
    std::println("Tri: {}", i);
    // Add points to array
    std::array<glm::vec3, 3> points;
    for (uint j = 0; j < 3; ++j) {
      glm::vec3 point = mMeshVertices[mMeshIndices[i*3+j]];
      // point *= 30.f; // Tyra
      // point *= 22.f; // Hollow Knight
      // point.x += mResolution / 2.f;
      // point.y += mResolution / 2.f;
      // point.z += mResolution / 2.f;

      // Bunnny
      // point *= 50.f;
      // point.x += mResolution / 1.8f;
      // point.y += mResolution / 10.f;
      // point.z += mResolution / 2.f;

      // Sponza
      // point *= mResolution * 0.0234375f;
      // point.x += mResolution / 2.f;
      // point.y += mResolution / 2.f;
      // point.z += mResolution / 2.f;

      // Green player
      point *= mResolution * 2;
      point.x += mResolution / 2.f;
      point.y += mResolution / 2.f;
      point.z += mResolution / 2.f;

      // Dragon
      // point += 1;
      // point *= 100;
      // point.x += mResolution / 2.f;
      // point.y += mResolution / 2.f;
      // point.z += mResolution / 2.f;

      points[j] = point;
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

    // Iterate over along the dominant axis traversing between the two lines
    for (float a = v3index(points[0], dominantAxisIndex); a <= v3index(points[2], dominantAxisIndex); ++a) {
      glm::vec3* l1Dir;
      glm::vec3* l1DirInv;
      if ((a > v3index(points[1], dominantAxisIndex)) || (v3index(lmDir, dominantAxisIndex) == 0)) {
        l1Dir = &mhDir;
        l1DirInv = &mhDirInv;
      }
      else {
        l1Dir = &lmDir;
        l1DirInv = &lmDirInv;
      }
      glm::vec3* l2Dir = &lhDir;
      glm::vec3* l2DirInv = &lhDirInv;

      float l1t = (a - v3index(points[1], dominantAxisIndex)) * v3index(*l1DirInv, dominantAxisIndex); // Med-high
      float l2t = (a - v3index(points[0], dominantAxisIndex)) * v3index(*l2DirInv, dominantAxisIndex); // Low-high

      glm::vec2 l1Pos = glm::vec2(v3index(points[1], nonDominantAxisIndices[0]), v3index(points[1], nonDominantAxisIndices[1])) + l1t * glm::vec2(v3index(*l1Dir, nonDominantAxisIndices[0]), v3index(*l1Dir, nonDominantAxisIndices[1]));
      glm::vec2 l2Pos = glm::vec2(v3index(points[0], nonDominantAxisIndices[0]), v3index(points[0], nonDominantAxisIndices[1])) + l2t * glm::vec2(v3index(*l2Dir, nonDominantAxisIndices[0]), v3index(*l2Dir, nonDominantAxisIndices[1]));

      glm::vec2 dir = l2Pos - l1Pos; // Dir from l1 to l2 intersection points
      glm::vec2 dirInv = 1.f/dir;

      glm::vec2 voxelPos = glm::floor(l1Pos);
      voxelPos.x -= glm::sign(dir.x);
      voxelPos.y -= glm::sign(dir.y);

      std::println("Dir: {}, {}, voxelPos: {}, {}, dirInvSigns: {}, {}, dirSigns: {}, {}", dir.x, dir.y, voxelPos.x, voxelPos.y, glm::sign(dirInv.x), glm::sign(dirInv.y), glm::sign(dir.x), glm::sign(dir.y));

      // Draw line
      while (voxelPos != glm::floor(l2Pos)) {
      std::println("voxelPos: {}, {}, l2pos: {}, {}, signDirInv: {}, {}", voxelPos.x, voxelPos.y, l2Pos.x, l2Pos.y, glm::sign(dirInv.x), glm::sign(dirInv.y));
      // while ( (((voxelPos.x <= l2Pos.x) * std::max(0.f, glm::sign(dirInv.x))) ||
      //          ((voxelPos.x >= l2Pos.x) * std::max(0.f, glm::sign(dirInv.x) * -1))) &&
      //         (((voxelPos.y <= l2Pos.y) * std::max(0.f, glm::sign(dirInv.y))) ||
      //          ((voxelPos.y >= l2Pos.y) * std::max(0.f, glm::sign(dirInv.y) * -1))) ) {
        float plane1 = voxelPos.x + std::max(0.f, glm::sign(dirInv.x));
        float plane2 = voxelPos.y + std::max(0.f, glm::sign(dirInv.y));

        if (plane1 == l2Pos.x && plane2 == l2Pos.y) { // If the destination point lies exactly on the planes
          voxelPos += glm::sign(dirInv);
          glm::vec3 v = (toVec3(a, voxelPos.x, voxelPos.y, dominantAxisIndex));
          v = glm::floor(v);
          if (!mVoxelGrid[v.x][v.y][v.z]) {
            ++mVoxelCount;
            insert(v, {glm::vec4(1, 1, 1, 0)});
            mVoxelGrid[v.x][v.y][v.z] = true;
          }
          break;
        }
        
        float t1 = (plane1 - l1Pos.x) * dirInv.x;
        float t2 = (plane2 - l1Pos.y) * dirInv.y;

        if (t1 < t2)
          voxelPos.x += glm::sign(dirInv.x);
        else
          voxelPos.y += glm::sign(dirInv.y);

        glm::vec3 v = (toVec3(a, voxelPos.x, voxelPos.y, dominantAxisIndex));
        v = glm::floor(v);
        if (!mVoxelGrid[v.x][v.y][v.z]) {
          ++mVoxelCount;
          insert(v, {glm::vec4(1, 1, 1, 0)});
          mVoxelGrid[v.x][v.y][v.z] = true;
        }
      }
    }

    if (mTriangleCount > 100) {
      ++countSinceLastPrint;
      if (countSinceLastPrint > progressDelay) {
        printProgressBar((i + 1) * triangleCountInv, "Voxelizing:");
        countSinceLastPrint = 0;
      }
    }
  }
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

