#include "RTVE.hpp"

// #include <thread_pool/thread_pool.h>

#include <unordered_map>

// #include "chunk.hpp"

int main(int argc, char* argv[]) {
  if (argc < 2) throw std::invalid_argument("No path provided");
  RTVE::Window& window = RTVE::Window::get();
  window.init("RTVE Demo");
  window.captureCursor();
  window.setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.f));

  RTVE::Camera camera;
  RTVE::Skybox skybox({ RTVE::Path("res/skybox/right.jpeg"),
                        RTVE::Path("res/skybox/left.jpeg"),
                        RTVE::Path("res/skybox/top.jpeg"),
                        RTVE::Path("res/skybox/bottom.jpeg"),
                        RTVE::Path("res/skybox/front.jpeg"),
                        RTVE::Path("res/skybox/back.jpeg")    });
  camera.attachSkybox(&skybox);

  // RTVE::Palette palette({ "sandbox/res/textures/grass_block_top.png",
  // RTVE::TexturePalette palette({ "sandbox/res/texturesNew/Grass_Top.png",
  //                         // "sandbox/res/textures/grass_block_side.png",
  //                         "sandbox/res/texturesNew/Grass_Side.png",
  //                         "sandbox/res/textures/dirt.png",
  //                         "sandbox/res/textures/water_still.png",
  //                         "sandbox/res/textures/oak_log_top.png",
  //                         "sandbox/res/textures/oak_log.png"  }, {8, 8},
  //                       { {2,2,2,2,2,2}, {0,1,2,2,2,2}, {3,3,3,3,3,3}, {4,5,4,4,4,4} });
  // RTVE::ColourPalette palette({ {glm::vec4(0.54f, 0.38f, 0.22f, 0.f)},
  //                               {glm::vec4(0.32f, 0.65f, 0.20f, 0.f)},
  //                               {glm::vec4(0.28f, 0.59f, 1.00f, 0.f)},
  //                               {glm::vec4(0.45f, 0.35f, 0.21f, 0.f)} });
  std::string palettePath = argv[1];
  palettePath.append(".pal");
  std::string modelPath = argv[1];
  modelPath.append(".vm8");
  RTVE::ColourPalette palette(palettePath);
  camera.attachColourPalette(&palette);

  RTVE::SparseVoxelDAG model(modelPath);
  camera.resizeIndicesBuffer(model.mIndices.size() * 8 * sizeof(uint32_t));
  // RTVE::SparseVoxelDAG model("sandbox/res/test.bin");
  // RTVE::SparseVoxelDAG model("sandbox/res/testC.bin");
  // RTVE::SparseVoxelDAG model("sandbox/res/highres.bin");
  camera.attachSparseVoxelDAG(&model);
  // model.generateDebugMesh();

  // model.print();

  // dp::thread_pool threadPool(4);

  // std::unordered_map<glm::ivec2, Chunk*> chunks;
  // std::vector<glm::ivec2> unattachedChunks;
  // std::vector<glm::ivec2> attachedChunks;
  // std::mutex chunksMutex;
  // int renderDistance = 2;
  // int renderDistance2 = renderDistance * renderDistance;
  // int blockRenderDistance = renderDistance * CHUNK_SIZE;
  // int blockRenderDistance2 = blockRenderDistance * blockRenderDistance;
  // for (glm::ivec2 chunkPos(-renderDistance, -renderDistance); chunkPos.x <= renderDistance; ++chunkPos.x)
  //   for (chunkPos.y = -renderDistance; chunkPos.y <= renderDistance; ++chunkPos.y)
  //     if (glm::length2(glm::vec2(chunkPos)) < renderDistance2) {
  //       chunks[chunkPos] = nullptr;
  //       threadPool.enqueue_detach([&chunks, &unattachedChunks, chunkPos, &chunksMutex]() {
  //         Chunk* ptr = new Chunk(chunkPos);
  //         {
  //           std::lock_guard<std::mutex> lock(chunksMutex);
  //           chunks[chunkPos] = ptr;
  //           unattachedChunks.push_back(chunkPos);
  //         }
  //       });
  //     }
  
  camera.mPos = glm::vec3(0, 0, 0);
  camera.setDirection(0, 0);

  glm::vec2 lastMousePos;

  window.mouseCallback = [&camera, &lastMousePos](glm::vec2 pOffset) {
    glm::vec2 offset(pOffset.x - lastMousePos.x, lastMousePos.y - pOffset.y);
    lastMousePos = pOffset;
    
    const float sensitivity = 0.1f;
    offset *= sensitivity;

    float pitch = camera.getPitch() + offset.y;
    if (pitch > 89.0f) pitch = 89.f;
    else if (pitch < -89.0f) pitch = -89.f;
    camera.setDirection(camera.getYaw() + offset.x, pitch);

    // std::println("Camera yaw: {}, pitch: {}", camera.getYaw(), camera.getPitch());
    // std::println("Camera direction: {}, {}, {}", camera.getDirectionVector().x, camera.getDirectionVector().y, camera.getDirectionVector().z);
    // std::println("Camera position: {}, {}, {}", camera.mPos.x, camera.mPos.y, camera.mPos.z);
  };
  window.framebufferSizeCallback = [&camera](glm::vec2 pSize) {
    camera.updateViewportSize(pSize);
  };
  // camera.updateViewportSize(window.getSize());

  float deltaTime, lastFrame, lastTimeFPSPrinted = 0.f;
  uint frames = 0u;
  bool debugRendering = false, fLastPressed = false, escLastPressed = false;
  while (!window.shouldWindowClose()) {
    // for (auto it = unattachedChunks.begin(); it != unattachedChunks.end();) {
    //   // camera.attachSparseVoxelDAG(&chunks.at(*it)->mSVDAG);
    //   attachedChunks.push_back(*it);
    //   {
    //     std::lock_guard<std::mutex> lock(chunksMutex);
    //     it = unattachedChunks.erase(it);
    //   }
    // }

    // glm::vec2 cameraChunkPos(camera.mPos.x / CHUNK_SIZE, camera.mPos.z / CHUNK_SIZE);
    // cameraChunkPos = glm::floor(cameraChunkPos);

    // for (auto it = attachedChunks.begin(); it != attachedChunks.end();) {
    //   auto chunk = chunks.find(*it);
    //   if (glm::distance2(glm::vec2(chunk->second->getChunkPos()), cameraChunkPos) > renderDistance2) {
    //     // camera.detachSparseVoxelDAG(&chunk->second->mSVDAG);
    //     delete chunk->second;
    //     {
    //       std::lock_guard<std::mutex> lock(chunksMutex);
    //       chunks.erase(chunk);
    //     }
    //     it = attachedChunks.erase(it);
    //   }
    //   else
    //     ++it;
    // }

    // for (glm::ivec2 chunkPos(cameraChunkPos.x - renderDistance, cameraChunkPos.y - renderDistance); chunkPos.x <= cameraChunkPos.x + renderDistance; ++chunkPos.x)
    //   for (chunkPos.y = cameraChunkPos.y - renderDistance; chunkPos.y <= cameraChunkPos.y + renderDistance; ++chunkPos.y)
    //     if (glm::distance2(glm::vec2(chunkPos), cameraChunkPos) < renderDistance2 && !chunks.contains(chunkPos)) {
    //       chunks[chunkPos] = nullptr;
    //       threadPool.enqueue_detach([&chunks, &unattachedChunks, chunkPos, &chunksMutex]() {
    //         Chunk* ptr = new Chunk(chunkPos);
    //         {
    //           std::lock_guard<std::mutex> lock(chunksMutex);
    //           chunks[chunkPos] = ptr;
    //           unattachedChunks.push_back(chunkPos);
    //         }
    //       });
    //     }

    float currentFrame = window.getTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    window.pollEvents();
    window.clear();

    if (debugRendering)
      camera.debugRender(window);
    camera.render();

    // Input - controls
    float cameraSpeed = 40.f * deltaTime;
    if (window.getKeyGLFW(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      cameraSpeed *= 2.f;
    if (window.getKeyGLFW(GLFW_KEY_SPACE) == GLFW_PRESS)
      camera.mPos.y += cameraSpeed;
    if (window.getKeyGLFW(GLFW_KEY_W) == GLFW_PRESS)
      camera.moveForward(cameraSpeed);
    if (window.getKeyGLFW(GLFW_KEY_S) == GLFW_PRESS)
      camera.moveBackward(cameraSpeed);
    if (window.getKeyGLFW(GLFW_KEY_A) == GLFW_PRESS)
      camera.moveLeft(cameraSpeed);
    if (window.getKeyGLFW(GLFW_KEY_D) == GLFW_PRESS)
      camera.moveRight(cameraSpeed);

    if (window.getKeyGLFW(GLFW_KEY_U) == GLFW_PRESS)
      camera.setLightPos(camera.mPos);

    if (window.getKeyGLFW(GLFW_KEY_F) == GLFW_PRESS && !fLastPressed) {
      fLastPressed = true;
      debugRendering = !debugRendering;
    }
    else if (window.getKeyGLFW(GLFW_KEY_F) == GLFW_RELEASE)
      fLastPressed = false;

    if (window.getKeyGLFW(GLFW_KEY_ESCAPE) == GLFW_PRESS && !escLastPressed) {
      if (window.isCursorCaptured())
        window.uncaptureCursor();
      else
        window.captureCursor();
      escLastPressed = true;
    }
    else if (window.getKeyGLFW(GLFW_KEY_ESCAPE) == GLFW_RELEASE)
      escLastPressed = false;

    window.swapBuffers();

    // Calculate FPS
    ++frames;
    if (window.getTime() - lastTimeFPSPrinted > 2) {
      std::println("FPS: {}", frames / (window.getTime() - lastTimeFPSPrinted));
      lastTimeFPSPrinted = window.getTime();
      frames = 0;
    }
  }

  // threadPool.wait_for_tasks();
  // for (auto it: chunks)
    // delete chunks.at(it.first);
  skybox.release();

  // model.releaseDebugMesh();
}

