#include "camera.hpp"

RTVE::Camera::Camera()
:mSVDAGShader("base/shaders/rtvShader.cs"), mScreenShader("base/shaders/textureShader.vs", "base/shaders/textureShader.fs"), mDebugShader("base/shaders/debugShader.vs", "base/shaders/debugShader.fs"), mWorldUp(0, 1, 0), mPos(0, 0, 0), mScreenSize(512, 512) {
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
  glBindAttribLocation(mSVDAGShader.getID(), 0, "SVDAGindices");
  glGenBuffers(1, &mSVDAGindicesSSBO);
  {
    std::vector<uint> data(1000000, 0);
    glNamedBufferData(mSVDAGindicesSSBO, data.size() * sizeof(uint), &data.at(0), GL_DYNAMIC_DRAW);
  }
  // Data buffer
  glBindAttribLocation(mSVDAGShader.getID(), 1, "SVDAGdata");
  glGenBuffers(1, &mSVDAGdataSSBO);
  {
    std::vector<VoxelData> data(1000, {glm::vec4(0, 0, 0, 0)});
    glNamedBufferData(mSVDAGdataSSBO, data.size() * sizeof(VoxelData), &data.at(0), GL_DYNAMIC_DRAW);
  }
  // Chunk Map buffer
  glBindAttribLocation(mSVDAGShader.getID(), 2, "ChunkMap");
  glGenBuffers(1, &mChunkMapSSBO);
  mChunkMap = std::vector<uint32_t>(1000, 0); // 1000 because uints are grouped into 2s(indices index, metadata index)
  glNamedBufferData(mChunkMapSSBO, mChunkMap.size() * sizeof(uint32_t), &mChunkMap.at(0), GL_DYNAMIC_DRAW);
  // Metadata buffer
  glBindAttribLocation(mSVDAGShader.getID(), 3, "Metadata");
  glGenBuffers(1, &mMetadataSSBO);
  mSVDAGMetadata = std::vector<uint32_t>(1000, 0); // 1000 because metadata consists of midpoint and resolution per svdag
  glNamedBufferData(mMetadataSSBO, mSVDAGMetadata.size() * sizeof(uint32_t), &mSVDAGMetadata.at(0), GL_DYNAMIC_DRAW);
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
  mSVDAGShader.setInt("uSVDAGSize", mSVDAG->getSize());
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mSVDAGShader.setMat4("uProjViewInv", glm::inverse(mProjection * view));
  mSVDAGShader.setFloat("uInverseNear", mInverseNear);
  mSVDAGShader.setFloat("uInverseFrustumDepth", mInverseFrustumDepth);
  mSVDAGShader.setFloat("uFar", mFar);
  mSVDAGShader.setInt("uMidpoint", mSVDAG->getMidpoint());
  mSVDAGShader.setVec2("uHalfResolutionInv", mHalfResolutionInv);

  // Bind SSBOs
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGindicesSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, mSVDAGindicesSSBO);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGdataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, mSVDAGdataSSBO);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mChunkMapSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mChunkMapSSBO);

  glBindBuffer(GL_SHADER_STORAGE_BUFFER, mSVDAGMetadataSSBO);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, mSVDAGMetadataSSBO);

  // Dispatch compute
  glDispatchCompute(mScreenSize.x / 30, mScreenSize.y / 30, 1);
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
}

void RTVE::Camera::debugRender(Window& pWindow) {
#ifdef _DEBUG
  mDebugShader.use();
  glm::mat4 view = glm::lookAt(mPos, mPos + mFront, mUp);
  mDebugShader.setMat4("uView", view);
  mDebugShader.setMat4("uProjection", mProjection);

  mSVDAG->drawDebug(&mDebugShader);
#endif
}

void RTVE::Camera::attachSparseVoxelDAG(SparseVoxelDAG* pSVDAG) {
  mSVDAGs.push_back(pSVDAG);
  mSVDAGShader.use();

  // Indices buffer ---------------------------------------
  // glBufferData(GL_SHADER_STORAGE_BUFFER, pSVDAG->mIndices.size() * (8*sizeof(uint32_t)), &pSVDAG->mIndices.at(0), GL_STATIC_DRAW);
  uint size = pSVDAG->mIndices.size() * 8 * sizeof(uint32_t);
  glNamedBufferSubData(mSVDAGindicesSSBO, mIndicesBufferSizeBytes, size, &pSVDAG->mIndices.at(0));
  mIndicesBufferSizeBytes += size;
  mIndicesBufferSize += pSVDAG->mIndices.size();

  // Data buffer ------------------------------------------
  // glBufferData(GL_SHADER_STORAGE_BUFFER, pSVDAG->mData.size() * sizeof(VoxelData), &pSVDAG->mData.at(0), GL_STATIC_DRAW);
  size = pSVDAG->mData.size() * sizeof(VoxelData);
  glNamedBufferSubData(mSVDAGdataSSBO, mDataBufferSize, size, &pSVDAG->mData.at(0));
  mDataBufferSize += size;

  // Metadata buffer --------------------------------------
  uint metadataIndex = mSVDAGMetadata.size();
  for (uint i = 0; i < mSVDAGMetadata.size() >> 1; ++i) {
    if (mSVDAGMetadata.at(i << 1) == pSVDAG.getMidpoint() && mSVDAGMetadata.at((i << 1) + 1) == pSVDAG.getSize()) {
      metadataIndex = i << 1;
      break;
    }
  }
  if (metadataIndex == mSVDAGMetadata.size()) {
    std::array<uint32_t, 2> metadata{pSVDAG.getMidpoint(), pSVDAG.getSize()};
    glNamedBufferSubData(mMetadataSSBO, mMetadataBufferSizeBytes, sizeof(metadata), &metadata.at(0));
    mMetadataBufferSizeBytes += sizeof(metadata);
  }

  // Chunk map buffer -------------------------------------
  // Update
  mChunkMap.at(mChunkMapSize << 1) = mIndicesBufferSize - pSVDAG->mIndices.size();
  mChunkMap.at(mChunkMapSize << 1 + 1) = metadataIndex;
  mChunkMapSize += 1;
  // Fill buffer
  glNamedBufferSubData(mChunkMapSSBO, 0, mChunkMap.size() * sizeof(uint32_t), &mChunkMap.at(0));
}

RTVE::Camera::~Camera() {
  glDeleteVertexArrays(1, &mScreenVAO);
  glDeleteBuffers(1, &mScreenVBO);
  glDeleteBuffers(1, &mSVDAGdataSSBO);
  glDeleteBuffers(1, &mSVDAGindicesSSBO);
  glDeleteBuffers(1, &mChunkMapSSBO);
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

