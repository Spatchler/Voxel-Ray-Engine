#include "camera.hpp"

GLenum glCheckError_(const char* file, int line) {
  GLenum errorCode;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    std::string error;
    switch (errorCode) {
      case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
      case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
      case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
      case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
      case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
      case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
      case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
    }
    std::println("{} | {} ({})", error, file, line);
  }
  return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

RTVE::Camera::Camera()
:mSkybox(NULL), mSVDAGColouredShader("base/shaders/rtvShaderOldColoured.comp"), mScreenShader("base/shaders/textureShader.vert", "base/shaders/textureShader.frag"), mSkyboxShader("base/shaders/skyboxShader.vert", "base/shaders/skyboxShader.frag"), mDebugShader("base/shaders/debugShader.vert", "base/shaders/debugShader.frag"), mWorldUp(0, 1, 0), mPos(0, 0, 0), mScreenSize(512, 512) {
  float vertices[] = {
    // positions  // texture coords
    -1.0f,  1.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 1.0f, 0.0f,
  };

  // Create screen VAO and VBO
  glGenVertexArrays(1, &mScreenVAO);
  glGenBuffers(1, &mScreenVBO);
  
  glBindVertexArray(mScreenVAO);

  glBindBuffer(GL_ARRAY_BUFFER, mScreenVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)0);

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (void*)(2 * sizeof(float)));

  glBindVertexArray(0);

  mInverseNear = 1.f/mNear;
  mInverseFrustumDepth = 1.f/(1.f/mFar - mInverseNear);

  // Create compute shader texture
  glGenTextures(1, &mComputeTexture);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mComputeTexture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mScreenSize.x, mScreenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
  
  glBindImageTexture(0, mComputeTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

  // Init SSBOs so they dont have to be reallocated
  // Indices buffer
  glCreateBuffers(1, &mSVDAGindicesSSBO);
  {
    ScopedTimer t("Allocating indices buffer");
    glNamedBufferStorage(mSVDAGindicesSSBO, 1000000000 * sizeof(uint32_t), NULL, GL_DYNAMIC_STORAGE_BIT);
  }
  // Texture Data buffer
  glCreateBuffers(1, &mSVDAGtextureDataSSBO);
  // {
  //   std::vector<TextureVoxelData> data(1, {0});
  //   glNamedBufferData(mSVDAGtextureDataSSBO, data.size() * sizeof(TextureVoxelData), &data.at(0), GL_STATIC_DRAW);
  // }
  // Colour Data buffer
  glCreateBuffers(1, &mSVDAGcolourDataSSBO);
  // {
  //   std::vector<ColourVoxelData> data(1, {glm::vec4(0, 0, 0, 0)});
  //   glNamedBufferData(mSVDAGcolourDataSSBO, data.size() * sizeof(ColourVoxelData), &data.at(0), GL_STATIC_DRAW);
  // }
  // Metadata buffer
  glCreateBuffers(1, &mMetadataSSBO);
  mSVDAGMetadata = std::vector<Metadata>(100, {0, 0, 0, 0, glm::tvec3<float>(0, 0, 0), 0});
  glNamedBufferData(mMetadataSSBO, mSVDAGMetadata.size() * sizeof(Metadata), &mSVDAGMetadata.at(0), GL_DYNAMIC_DRAW);
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
  mScreenSize = pSize;
  mProjection = glm::perspective(glm::radians(45.0f), pSize.x / pSize.y, mNear, mFar);
  mHalfResolutionInv = 2.f / pSize;

  // Update compute texture parameters
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mComputeTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mScreenSize.x, mScreenSize.y, 0, GL_RGBA, GL_FLOAT, NULL);
  
  glBindImageTexture(0, mComputeTexture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
}

void RTVE::Camera::render() {
  mSVDAGColouredShader.use();
  mSVDAGColouredShader.setVec3("uCamPos", mPos);
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mSVDAGColouredShader.setMat4("uProjViewInv", glm::inverse(mProjection * view));
  mSVDAGColouredShader.setFloat("uInverseNear", mInverseNear);
  mSVDAGColouredShader.setFloat("uInverseFrustumDepth", mInverseFrustumDepth);
  mSVDAGColouredShader.setFloat("uFar", mFar);
  mSVDAGColouredShader.setVec2("uHalfResolutionInv", mHalfResolutionInv);
  mSVDAGColouredShader.setUInt("uNumOctrees", mSVDAGs.size());
  mSVDAGColouredShader.setFloat("uTime", glfwGetTime());

  // Bind palette
  // mSVDAGShader.setInt("texArray", 0);
  // glActiveTexture(GL_TEXTURE0);
  // glBindTexture(GL_TEXTURE_2D_ARRAY, mTexturePalette->getTexID());

  // Bind SSBOs
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSVDAGcolourDataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mMetadataSSBO);

  // Dispatch compute
  glDispatchCompute(glm::ceil(mScreenSize.x / 30.f), glm::ceil(mScreenSize.y / 30.f), 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  // Bind texture
  mScreenShader.use();
  mScreenShader.setInt("tex", 0);
  mScreenShader.setVec2("uScreenSize", mScreenSize);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mComputeTexture);

  // Render quad
  glBindVertexArray(mScreenVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindVertexArray(0);

  // Render skybox
  if (mSkybox) {
    mSkyboxShader.use();
    mSkyboxShader.setInt("skybox", 0);
    mSkyboxShader.setMat4("uProjection", mProjection);
    mSkyboxShader.setMat4("uView", glm::mat4(glm::mat3(view)));
    mSkybox->draw();
  }
}

void RTVE::Camera::debugRender(Window& pWindow) {
#ifdef _DEBUG
  mDebugShader.use();
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mDebugShader.setMat4("uView", view);
  mDebugShader.setMat4("uProjection", mProjection);

  for (SparseVoxelDAG* p: mSVDAGs)
    p->drawDebug(&mDebugShader);
#endif
}

void RTVE::Camera::attachTexturePalette(TexturePalette* pPalette) {
  mTexturePalette = pPalette;
  glNamedBufferData(mSVDAGtextureDataSSBO, pPalette->getSize() * sizeof(TextureVoxelData), pPalette->getStart(), GL_STATIC_DRAW);
}

void RTVE::Camera::attachColourPalette(ColourPalette* pPalette) {
  mColourPalette = pPalette;
  glNamedBufferData(mSVDAGcolourDataSSBO, pPalette->getSize() * sizeof(ColourVoxelData), pPalette->getStart(), GL_STATIC_DRAW);
}

void RTVE::Camera::attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG) {
  // Indices buffer ---------------------------------------
  uint64_t size = pSVDAG->mIndices.size() * 8 * 4;
  glCheckError();
  glNamedBufferSubData(mSVDAGindicesSSBO, mIndicesBufferSizeBytes, size, &pSVDAG->mIndices.at(0));
  mIndicesBufferSizeBytes += size;
  glCheckError();

  // Metadata buffer --------------------------------------
  // mSVDAGMetadata.at(mMetadataBufferSize) = {mIndicesBufferSize, pSVDAG->getMidpoint(), pSVDAG->getSize(), 0, pSVDAG->getTranslation(), mColourPalette->getSize()};
  mSVDAGMetadata.at(mMetadataBufferSize) = {mIndicesBufferSize, pSVDAG->getMidpoint(), pSVDAG->getSize(), 0, pSVDAG->getTranslation(), pSVDAG->getMaxDepth()};
  glNamedBufferSubData(mMetadataSSBO, mMetadataBufferSizeBytes, sizeof(Metadata), &mSVDAGMetadata.at(mMetadataBufferSize));
  ++mMetadataBufferSize;
  
  // Update indices buffer size after so we dont have to subtract when writing metadata
  mIndicesBufferSize += pSVDAG->mIndices.size();
  mMetadataBufferSizeBytes += sizeof(Metadata);

  mSVDAGs.push_back(pSVDAG); // Push back at the end so num octrees uniform only changes after data is ready
}

void RTVE::Camera::detachSparseVoxelDAG(SparseVoxelDAG* pSVDAG) {
  const auto it = std::find(mSVDAGs.begin(), mSVDAGs.end(), pSVDAG);
  if (it == mSVDAGs.end()) return;
  mSVDAGMetadata.erase(mSVDAGMetadata.begin() + std::distance(mSVDAGs.begin(), it));
  mSVDAGs.erase(it);
  --mMetadataBufferSize;
  glNamedBufferSubData(mMetadataSSBO, 0, sizeof(Metadata) * mMetadataBufferSize, &mSVDAGMetadata.at(0));
}

void RTVE::Camera::attachSkybox(Skybox* pSkybox) {
  mSkybox = pSkybox;
}

void RTVE::Camera::detachSkybox() {
  mSkybox = NULL;
}

RTVE::Camera::~Camera() {
  glDeleteVertexArrays(1, &mScreenVAO);
  glDeleteBuffers(1, &mScreenVBO);
  glDeleteBuffers(1, &mSVDAGindicesSSBO);
  glDeleteBuffers(1, &mSVDAGcolourDataSSBO);
  glDeleteBuffers(1, &mSVDAGtextureDataSSBO);
  glDeleteBuffers(1, &mMetadataSSBO);
}

void RTVE::Camera::updateVectors() {
  glm::vec3 front;
  front.x = std::cos(glm::radians(mYaw)) * std::cos(glm::radians(mPitch));
  front.y = std::sin(glm::radians(mPitch));
  front.z = std::sin(glm::radians(mYaw)) * std::cos(glm::radians(mPitch));
  mFront = glm::normalize(front);
  mRight = glm::normalize(glm::cross(mFront, mWorldUp));
  mUp = glm::normalize(glm::cross(mRight, mFront));
}

