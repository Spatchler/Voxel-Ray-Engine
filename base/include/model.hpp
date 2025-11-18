#pragma once

#include "SparseVoxelDAG.hpp"

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

    std::vector<glm::vec3> mVertices;
    std::vector<uint> mIndices;

    std::vector<std::vector<std::vector<bool>>> mVoxelGrid;
    uint mResolution;

    std::string mDirectory;
  };
}

