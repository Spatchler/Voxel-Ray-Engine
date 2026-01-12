#pragma once

#include "window.hpp"
#include "shader.hpp"
#include "computeShader.hpp"
#include "SparseVoxelDAG.hpp"

namespace RTVE {
  struct Node {
    glm::vec3 origin;
    uint32_t index;
  };

  // Enabe tight packing
  #pragma pack(1)
  struct Metadata {
    uint32_t indicesIndex, midpoint, size;
    float paddingA;
    glm::tvec3<float> translation;
    float paddingB;
    // uint8_t c[4]; // 8
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

    void attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG);

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

    ComputeShader mSVDAGShader;
    Shader mScreenShader;
    uint mComputeTexture;
    uint mScreenVAO, mScreenVBO;
    uint mIndicesBufferSize = 0, mMetadataBufferSize = 0;
    uint mIndicesBufferSizeBytes = 0, mDataBufferSizeBytes = 0, mMetadataBufferSizeBytes = 0;
    uint mSVDAGindicesSSBO, mSVDAGdataSSBO, mMetadataSSBO;
#ifdef _DEBUG
    Shader mDebugShader;
#endif
  };
}

