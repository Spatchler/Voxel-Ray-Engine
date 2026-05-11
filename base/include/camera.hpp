#pragma once

#include "window.hpp"
#include "shader.hpp"
#include "computeShader.hpp"
#include "SparseVoxelDAG.hpp"
#include "skybox.hpp"
#include "palette.hpp"

namespace RTVE {
  struct Node {
    glm::vec3 origin;
    uint32_t index;
  };

  #pragma pack(1)
  struct Metadata {
    uint32_t indicesIndex, midpoint, size, paletteIndex;
    glm::tvec3<float> translation;
    uint32_t paletteSize;
  };
  #pragma pack()

  class Camera {
  public:
    Camera();

    void setDirection(float pYaw, float pPitch);
    void setYaw(float pYaw);
    void setPitch(float pPitch);
    float getYaw();
    float getPitch();
    glm::vec3 getDirectionVector();

    void moveForward(float pSpeed);
    void moveBackward(float pSpeed);
    void moveLeft(float pSpeed);
    void moveRight(float pSpeed);

    void updateViewportSize(const glm::vec2& pSize);

    void render();
    void debugRender(Window& pWindow);
    
    void attachTexturePalette(TexturePalette* pPalette);
    void attachColourPalette(ColourPalette* pPalette);
    void attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG);
    void detachSparseVoxelDAG(SparseVoxelDAG* pSVDAG);
    void attachSkybox(Skybox* pSkybox);
    void detachSkybox();

    ~Camera();

    glm::vec3 mPos;
  private:
    void updateVectors();

    glm::vec3 mFront, mUp, mRight, mWorldUp = {0, 0, 0};
    float mYaw, mPitch = 0.f;

    float mNear = 0.1f;
    float mFar = 10000.f;
    float mInverseNear;
    float mInverseFrustumDepth;
    glm::vec2 mHalfResolutionInv;
    glm::mat4 mProjection;

    glm::ivec2 mScreenSize;

    std::vector<SparseVoxelDAG*> mSVDAGs;
    std::vector<Metadata> mSVDAGMetadata;

    TexturePalette* mTexturePalette;
    ColourPalette* mColourPalette;
    Skybox* mSkybox;

    ComputeShader mSVDAGTexturedShader, mSVDAGColouredShader;
    Shader mScreenShader, mSkyboxShader;
    uint mComputeTexture;
    uint mScreenVAO, mScreenVBO;
    uint mIndicesBufferSize = 0, mMetadataBufferSize = 0, mColourDataBufferSize = 0;
    uint64_t mIndicesBufferSizeBytes = 0;
    uint mColourDataBufferSizeBytes = 0, mMetadataBufferSizeBytes = 0;
    uint mSVDAGindicesSSBO, mSVDAGtextureDataSSBO, mSVDAGcolourDataSSBO, mMetadataSSBO;
#ifdef _DEBUG
    Shader mDebugShader;
#endif
  };
}

