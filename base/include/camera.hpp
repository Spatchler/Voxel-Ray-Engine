#pragma once

#include "window.hpp"
#include "shader.hpp"
#include "world.hpp"

namespace RTVE {
  class Camera {
  public:
    Camera();

    void setDirection(float pYaw, float pPitch);
    void setPos(glm::vec3 pPos);

    void render(Window& pWindow);

    void attachSVO(SparseVoxelOctree* pSVO);
  private:
    glm::vec3 mPos, mFront, mUp, mRight, mWorldUp;
    
    Shader mShader;
    uint mVAO, mVBO;
    uint mSVOSSBO;
  };
}

