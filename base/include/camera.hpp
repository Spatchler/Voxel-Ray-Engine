#pragma once

#include "window.hpp"
#include "shader.hpp"
#include "computeShader.hpp"
#include "SparseVoxelDAG.hpp"

namespace RTVE {
  struct Node {
    glm::vec3 origin;
    uint index;
  };

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

    SparseVoxelDAG* mSVDAG;
    
    ComputeShader mSVDAGShader;
    Shader mScreenShader;
    uint mComputeTexture;
    uint mScreenVAO, mScreenVBO;
    uint mSVDAGindicesSSBO, mSVDAGdataSSBO;
#ifdef _DEBUG
    Shader mDebugShader;
#endif
  };
}

