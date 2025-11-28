#pragma once

#include "SparseVoxelDAG.hpp"
#include "helpers.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

namespace RTVE {
  class SVDAGModel: public SparseVoxelDAG {
  public:
    SVDAGModel(const std::string& pPath, uint pResolution);
  private:
    void loadModel(const std::string& pPath);
    void processNode(aiNode* pNode, const aiScene* pScene);
    void processMesh(aiMesh* pMesh, const aiScene* pScene);

    static glm::vec3 toVec3(float a, float b, float c, uint8_t pDominantAxisIndex);

    std::vector<glm::vec3> mMeshVertices;
    std::vector<uint> mMeshIndices;

    std::vector<std::vector<std::vector<bool>>> mVoxelGrid;
    uint mResolution;

    uint mVoxelCount;
    uint mTriangleCount;

    std::string mDirectory;
  };
}

