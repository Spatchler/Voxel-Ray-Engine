#include "model.hpp"

RTVE::SVDAGModel::SVDAGModel(const std::string& pPath, uint pResolution)
:SparseVoxelDAG(pResolution), mResolution(pResolution) {
  loadModel(pPath);

  // Init mVoxelGrid
  mVoxelGrid = std::vector<std::vector<std::vector<bool>>>(pResolution, std::vector<std::vector<bool>>(pResolution, std::vector<bool>(pResolution, false)));
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
  for (uint i = 0; i < mIndices.size() / 3; ++i) { // Every triangle
    // Iterate over the grid
    glm::ivec3 vec;
    for (; vec.y < mResolution; ++vec.y) {
      for (; vec.x < mResolution; ++vec.x) {
        for (; vec.z < mResolution; ++vec.z) {
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
  uint verticesSize = mVertices.size();

  for (uint i = 0; i < pMesh->mNumVertices; ++i) {
    glm::vec3 pos;

    pos.x = pMesh->mVertices[i].x;
    pos.y = pMesh->mVertices[i].y;
    pos.z = pMesh->mVertices[i].z;

    mVertices.push_back(pos);
  }

  for (uint i = 0; i < pMesh->mNumFaces; ++i) {
    aiFace face = pMesh->mFaces[i];
    for (uint j = 0; j < face.mNumIndices; ++j)
      mIndices.push_back(face.mIndices[j] + verticesSize);
  }
}

