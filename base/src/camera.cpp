#include "camera.hpp"

RTVE::Camera::Camera()
:mDAGShader("base/shaders/rtvShader.vs", "base/shaders/rtvShader.fs"), mDebugShader("base/shaders/debugShader.vs", "base/shaders/debugShader.fs"), mWorldUp(0, 1, 0), mPos(0, 0, 0) {
  float vertices[] {
    -1.0f,  1.0f,
    -1.0f, -1.0f,
     1.0f,  1.0f,
     1.0f, -1.0f
  };

  glGenVertexArrays(1, &mScreenVAO);
  glGenBuffers(1, &mScreenVBO);
  
  glBindVertexArray(mScreenVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mScreenVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);

  glBindVertexArray(0);

  mInverseNear = 1.f/mNear;
  mInverseFrustumDepth = 1.f/(1.f/mFar - mInverseNear);
}

void RTVE::Camera::setDirection(float pYaw, float pPitch) {
  mYaw = pYaw;
  mPitch = pPitch;
  updateVectors();
}

void RTVE::Camera::setYaw(float pYaw) {
  mYaw = pYaw;
  updateVectors();
}

void RTVE::Camera::setPitch(float pPitch) {
  mPitch = pPitch;
  updateVectors();
}

float RTVE::Camera::getYaw() {
  return mYaw;
}
float RTVE::Camera::getPitch() {
  return mPitch;
}

glm::vec3 RTVE::Camera::getDirectionVector() {
  return mFront;
}

void RTVE::Camera::moveForward(float pSpeed) {
  mPos += pSpeed * mFront;
}
void RTVE::Camera::moveBackward(float pSpeed) {
  mPos -= pSpeed * mFront;
}
void RTVE::Camera::moveLeft(float pSpeed) {
  mPos -= glm::normalize(glm::cross(mFront, mUp)) * pSpeed;
}
void RTVE::Camera::moveRight(float pSpeed) {
  mPos += glm::normalize(glm::cross(mFront, mUp)) * pSpeed;
}

void RTVE::Camera::updateViewportSize(const glm::vec2& pSize) {
  mProjection = glm::perspective(glm::radians(45.0f), pSize.x / pSize.y, mNear, mFar);
  mHalfResolutionInv = 2.f / pSize;
}

void RTVE::Camera::render() {
  mDAGShader.use();
  mDAGShader.setVec3("uCamPos", mPos);
  mDAGShader.setInt("uSVDAGSize", mAttachedSVDAG->getSize());
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mDAGShader.setMat4("uProjViewInv", glm::inverse(mProjection * view));
  mDAGShader.setFloat("uInverseNear", mInverseNear);
  mDAGShader.setFloat("uInverseFrustumDepth", mInverseFrustumDepth);
  mDAGShader.setFloat("uFar", mFar);
  mDAGShader.setInt("uMidpoint", mAttachedSVDAG->getMidpoint());
  mDAGShader.setVec2("uHalfResolutionInv", mHalfResolutionInv);

  // Bind SSBO
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGindicesSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGdataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSVDAGdataSSBO);

  // Render
  glBindVertexArray(mScreenVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  // Unbind SSBO
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGdataSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void RTVE::Camera::debugRender(Window& pWindow) {
  mDebugShader.use();
  glm::mat4 projection = glm::perspective(glm::radians(45.0f), pWindow.getSize().x / pWindow.getSize().y, 0.1f, 10000.0f);
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::scale(model, glm::vec3(mAttachedSVDAG->getSize(), mAttachedSVDAG->getSize(), mAttachedSVDAG->getSize()));
  mDebugShader.setMat4("uModel", model);
  mDebugShader.setMat4("uView", view);
  mDebugShader.setMat4("uProjection", projection);

  mAttachedSVDAG->drawDebug();
}

void RTVE::Camera::attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG) {
  mAttachedSVDAG = pSVDAG;

  // Indices buffer --------------------------------------
  mDAGShader.use();
  // Bind buffer
  glBindAttribLocation(mDAGShader.getID(), 0, "SVDAGindices");
  glGenBuffers(1, &mSVDAGindicesSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGindicesSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);
  // Fill buffer
  glBufferData(GL_SHADER_STORAGE_BUFFER, pSVDAG->mIndices.size() * (8*sizeof(uint)), &pSVDAG->mIndices.at(0), GL_STATIC_DRAW);
  // Unbind buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

  // Data buffer -----------------------------------------
  mDAGShader.use();
  // Bind buffer
  glBindAttribLocation(mDAGShader.getID(), 1, "SVDAGdata");
  glGenBuffers(1, &mSVDAGdataSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGdataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSVDAGdataSSBO);
  // Fill buffer
  glBufferData(GL_SHADER_STORAGE_BUFFER, pSVDAG->mData.size() * sizeof(VoxelData), &pSVDAG->mData.at(0), GL_STATIC_DRAW);
  // Unbind buffer
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGdataSSBO);
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

RTVE::Camera::~Camera() {
  glDeleteVertexArrays(1, &mScreenVAO);
  glDeleteBuffers(1, &mScreenVBO);
}

void RTVE::Camera::updateVectors() {
  glm::vec3 front;
  front.x = cos(glm::radians(mYaw)) * cos(glm::radians(mPitch));
  front.y = sin(glm::radians(mPitch));
  front.z = sin(glm::radians(mYaw)) * cos(glm::radians(mPitch));
  mFront = glm::normalize(front);
  mRight = glm::normalize(glm::cross(mFront, mWorldUp));
  mUp = glm::normalize(glm::cross(mRight, mFront));
}

