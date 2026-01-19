#include "camera.hpp"

RTVE::Camera::Camera()
:mSkybox(NULL), mSVDAGShader("base/shaders/rtvShader.comp"), mScreenShader("base/shaders/textureShader.vert", "base/shaders/textureShader.frag"), mSkyboxShader("base/shaders/skyboxShader.vert", "base/shaders/skyboxShader.frag"), mDebugShader("base/shaders/debugShader.vert", "base/shaders/debugShader.frag"), mWorldUp(0, 1, 0), mPos(0, 0, 0), mScreenSize(512, 512) {
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
  mSVDAGShader.use();
  // Indices buffer
  glBindAttribLocation(mSVDAGShader.getID(), 0, "SVDAGindicesSSBO");
  glCreateBuffers(1, &mSVDAGindicesSSBO);
  {
    // std::vector<uint32_t> data(10000000, 0);
    std::vector<uint32_t> data(100000000, 0);
    glNamedBufferData(mSVDAGindicesSSBO, data.size() * sizeof(uint), &data.at(0), GL_DYNAMIC_DRAW);
  }
  // Data buffer
  glBindAttribLocation(mSVDAGShader.getID(), 1, "SVDAGdataSSBO");
  glCreateBuffers(1, &mSVDAGdataSSBO);
  {
    // std::vector<VoxelData> data(1000, {glm::vec4(0, 0, 0, 0)});
    std::vector<VoxelData> data(1000, {0});
    glNamedBufferData(mSVDAGdataSSBO, data.size() * sizeof(VoxelData), &data.at(0), GL_DYNAMIC_DRAW);
  }
  // Metadata buffer
  glBindAttribLocation(mSVDAGShader.getID(), 2, "MetadataSSBO"); // TODO: Add data indices offset
  glCreateBuffers(1, &mMetadataSSBO);
  mSVDAGMetadata = std::vector<Metadata>(100000, {0, 0, 0, 0, glm::tvec3<float>(0, 0, 0), 0});
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
  mSVDAGShader.use();
  mSVDAGShader.setVec3("uCamPos", mPos);
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mSVDAGShader.setMat4("uProjViewInv", glm::inverse(mProjection * view));
  mSVDAGShader.setFloat("uInverseNear", mInverseNear);
  mSVDAGShader.setFloat("uInverseFrustumDepth", mInverseFrustumDepth);
  mSVDAGShader.setFloat("uFar", mFar);
  mSVDAGShader.setVec2("uHalfResolutionInv", mHalfResolutionInv);
  mSVDAGShader.setUInt("uNumOctrees", mSVDAGs.size());

  // Bind palette
  mSVDAGShader.setInt("texArray", 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D_ARRAY, mPalette->getTexID());

  // Bind SSBOs
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGindicesSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGdataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSVDAGdataSSBO);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mMetadataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mMetadataSSBO);

  // Dispatch compute
  glDispatchCompute(mScreenSize.x / 30.f, mScreenSize.y / 30.f, 1);
  glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

  // Bind texture
  mScreenShader.use();
  mScreenShader.setInt("tex", 0);
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

uint RTVE::Camera::attachPalette(Palette* pPalette) {
  mPalette = pPalette;
  uint size = pPalette->getSize() * sizeof(VoxelData);
  glNamedBufferSubData(mSVDAGdataSSBO, mDataBufferSizeBytes, size, pPalette->getStart());
  mDataBufferSizeBytes += size;
  mDataBufferSize += pPalette->getSize();
  return mDataBufferSize - pPalette->getSize();
}

void RTVE::Camera::attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG, uint32_t pPaletteOffset, uint32_t pPaletteSize) {
  mSVDAGShader.use();

  // Indices buffer ---------------------------------------
  uint size = pSVDAG->mIndices.size() * 8 * sizeof(uint32_t);
  glNamedBufferSubData(mSVDAGindicesSSBO, mIndicesBufferSizeBytes, size, &pSVDAG->mIndices.at(0));
  mIndicesBufferSizeBytes += size;

  // Metadata buffer --------------------------------------
  mSVDAGMetadata.at(mMetadataBufferSize) = {mIndicesBufferSize, pSVDAG->getMidpoint(), pSVDAG->getSize(), pPaletteOffset, pSVDAG->getTranslation(), pPaletteSize};
  glNamedBufferSubData(mMetadataSSBO, mMetadataBufferSizeBytes, sizeof(Metadata), &mSVDAGMetadata.at(mMetadataBufferSize));
  ++mMetadataBufferSize;
  
  // Update indices buffer size after so we dont have to subtract when writing metadata
  mIndicesBufferSize += pSVDAG->mIndices.size();
  mMetadataBufferSizeBytes += sizeof(Metadata);

  mSVDAGs.push_back(pSVDAG); // Push back at the end so num octrees uniform only changes after data is ready
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
  glDeleteBuffers(1, &mSVDAGdataSSBO);
  glDeleteBuffers(1, &mMetadataSSBO);
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

