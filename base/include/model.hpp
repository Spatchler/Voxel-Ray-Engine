#pragma once

#include "SparseVoxelDAG.hpp"
#include "helpers.hpp"
#include "shader.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

namespace RTVE {
  static int DDARound(float n) {
    if (n - (int)n < 0.5)
      return (int)n;
    return (int)(n + 1);
  }

  class SVDAGModel: public SparseVoxelDAG {
  public:
    SVDAGModel(const std::string& pPath, uint pResolution);

    void drawDebug(Shader* pShader) override;

    void generateModelDebugMesh();
    void releaseModelDebugMesh();
  protected:
    void loadModel(const std::string& pPath);
    void processNode(aiNode* pNode, const aiScene* pScene);
    void processMesh(aiMesh* pMesh, const aiScene* pScene);

    static glm::vec3 toVec3(float a, float b, float c, uint8_t pDominantAxisIndex);

    void drawLine2(uint8_t pDominantAxisIndex, float pDominantAxisValue, const glm::vec2& pStart, const glm::vec2& pEnd, const glm::vec2& pDir, const glm::vec2& pDirInv);

    void generateDebugLineIndices();

    std::vector<glm::vec3> mMeshVertices;
    std::vector<uint> mMeshIndices;

    std::vector<std::vector<std::vector<bool>>> mVoxelGrid;
    uint mResolution;

    uint mVoxelCount;
    uint mTriangleCount;

    std::string mDirectory;
#ifdef _DEBUG
    std::vector<uint> mDebugLineIndices;
    uint mDebugLineIndicesCount = 0;
    uint mModelDebugVAO, mModelDebugVBO, mModelDebugEBO;
#endif
  };
}

