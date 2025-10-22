#include "camera.hpp"

RTVE::Camera::Camera()
:mShader("base/res/shaders/rtvShader.vs", "base/res/shaders/rtvShader.fs"), mWorldUp(0, 1, 0) {
  float vertices[] {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f
  };

  glGenVertexArrays(1, &mVAO);
  glGenBuffers(1, &mVBO);
  
  glBindVertexArray(mVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

  glBindVertexArray(0);
}

void RTVE::Camera::setDirection(float pYaw, float pPitch) {
  glm::vec3 front;
  front.x = cos(glm::radians(pYaw)) * cos(glm::radians(pPitch));
  front.y = sin(glm::radians(pPitch));
  front.z = sin(glm::radians(pYaw)) * cos(glm::radians(pPitch));
  mFront = glm::normalize(front);
  mRight = glm::normalize(glm::cross(mFront, mWorldUp));
  mUp = glm::normalize(glm::cross(mRight, mFront));
}

void RTVE::Camera::setPos(glm::vec3 pPos) {
  mPos = pPos;
}

void RTVE::Camera::render(Window& pWindow) {
  mShader.use();
  mShader.setVec3("camPos", mPos);
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), pWindow.getSize().x / pWindow.getSize().y, 0.1f, 100.0f);
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mShader.setMat4("projViewInv", glm::inverse(projection * view));

  // Bind SSBO
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVOSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSVOSSBO);

  // Render
  glBindVertexArray(mVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  // Unbind SSBO
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RTVE::Camera::attachSVO(SparseVoxelOctree* pSVO) {
  mShader.use();
  // Bind buffer
  glBindAttribLocation(mShader.getID(), 1, "SVO");
  glGenBuffers(1, &mSVOSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVOSSBO);
  // Fill buffer
  glBufferData(GL_SHADER_STORAGE_BUFFER, pSVO->mNodes.size() * sizeof(Node), &pSVO->mNodes.at(0), GL_STATIC_DRAW);
  // Unbind buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVOSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

