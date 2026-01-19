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

  // Enabe tight packing
  #pragma pack(1)
  struct Metadata {
    uint32_t indicesIndex, midpoint, size, paletteIndex;
    glm::tvec3<float> translation;
    uint32_t paletteSize;
  };
  // Reset packing
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
    
    uint attachPalette(Palette* pPalette);
    void attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG, uint32_t pPaletteOffset, uint32_t pPaletteSize);
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

    Palette* mPalette;
    Skybox* mSkybox;

    ComputeShader mSVDAGShader;
    Shader mScreenShader, mSkyboxShader;
    uint mComputeTexture;
    uint mScreenVAO, mScreenVBO;
    uint mIndicesBufferSize = 0, mMetadataBufferSize = 0, mDataBufferSize = 0;
    uint mIndicesBufferSizeBytes = 0, mDataBufferSizeBytes = 0, mMetadataBufferSizeBytes = 0;
    uint mSVDAGindicesSSBO, mSVDAGdataSSBO, mMetadataSSBO;
#ifdef _DEBUG
    Shader mDebugShader;
#endif
  };
}

