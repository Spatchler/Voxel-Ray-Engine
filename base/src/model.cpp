#include "model.hpp"

RTVE::SVDAGModel::SVDAGModel(const std::string& pPath, uint pResolution)
:SparseVoxelDAG(pResolution), mResolution(pResolution) {
  // Init mVoxelGrid
  mVoxelGrid = std::vector<std::vector<std::vector<bool>>>(pResolution, std::vector<std::vector<bool>>(pResolution, std::vector<bool>(pResolution, false)));
  
  loadModel(pPath);
}

void RTVE::SVDAGModel::loadModel(const std::string& pPath) {
  Assimp::Importer importer;

  const aiScene* scene = importer.ReadFile(pPath, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    std::println("ERROR::ASSIMP::{0}", importer.GetErrorString());
    return;
  }

  mDirectory = pPath.substr(0, pPath.find_last_of('/'));

  processNode(scene->mRootNode, scene);

  // For every triangle iterate over the grid
  for (uint i = 0; i < mMeshIndices.size() / 3; ++i) { // Every triangle
    // Iterate over the grid
    glm::vec3 point = mMeshVertices[mMeshIndices[i*3]];
    std::println("point: {}, {}, {}", point.x, point.y, point.z);
    point *= 11;
    point.x += mResolution / 2;
    point.z += mResolution / 2;
    glm::ivec3 vec;
    vec.x = floor(point.x);
    vec.y = floor(point.y);
    vec.z = floor(point.z);
    std::println("vec: {}, {}, {}", vec.x, vec.y, vec.z);
    mVoxelGrid[vec.x][vec.y][vec.z] = true;
    // glm::vec3 vec(0, 0, 0);
    // for (vec.y = 0; vec.y < mResolution; ++vec.y) {
    //   for (vec.x = 0; vec.x < mResolution; ++vec.x) {
    //     for (vec.z = 0; vec.z < mResolution; ++vec.z) {
    //       // Check if triangle and cube intersect
    //       glm::vec3 point = mMeshVertices[mMeshIndices[i*3]];
    //       point *= 100;
    //       point.y += 5;
    //       // std::println("point: {}, {}, {}", point.x, point.y, point.z);
    //       if ((point.x > vec.x && point.x < vec.x+1) && (point.y > vec.y && point.y < vec.y+1) && (point.z > vec.z && point.z < vec.z+1)) {
    //         mVoxelGrid[vec.y][vec.x][vec.z] = true;
    //       }
    //     }
    //   }
    // }
  }

  mData.push_back({glm::vec4(0.1, 0.1, 0.1, 0)});

  glm::ivec3 vec(0, 0, 0);
  for (vec.x = 0; vec.x < mResolution; ++vec.x) {
    for (vec.y = 0; vec.y < mResolution; ++vec.y) {
      for (vec.z = 0; vec.z < mResolution; ++vec.z) {
        std::println("vec: {}, {}, {}", vec.x, vec.y, vec.z);
        if (mVoxelGrid.at(vec.x).at(vec.y).at(vec.z)) {
          std::println("inserting");
          insert(vec, {glm::vec4(1, 1, 1, 0)});
        }
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

