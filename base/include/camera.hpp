#pragma once

#include "window.hpp"
#include "shader.hpp"
#include "world.hpp"

namespace RTVE {
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

    void render(Window& pWindow);

    void attachSVO(SparseVoxelOctree* pSVO);
    
    glm::vec3 mPos;
  private:
    void updateVectors();
    glm::vec3 mFront, mUp, mRight, mWorldUp = {0, 0, 0};
    float mYaw, mPitch = 0.f;

    SparseVoxelOctree* mAttachedSVO;
    
    Shader mShader;
    uint mVAO, mVBO;
    uint mSVOindicesSSBO, mSVOdataSSBO;
  };
}

