#include "RTVE.hpp"

#include "chunk.hpp"

int main() {
  RTVE::Window& window = RTVE::Window::get();
  window.init("RTVE Demo");
  window.captureCursor();
  window.setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.f));

  RTVE::Camera camera;
  RTVE::Skybox skybox({ "sandbox/res/skybox/right.jpeg",
                        "sandbox/res/skybox/left.jpeg",
                        "sandbox/res/skybox/top.jpeg",
                        "sandbox/res/skybox/bottom.jpeg",
                        "sandbox/res/skybox/front.jpeg",
                        "sandbox/res/skybox/back.jpeg"    });
  camera.attachSkybox(&skybox);

  // RTVE::SparseVoxelDAG model("sandbox/res/outS.bin");
  // RTVE::SparseVoxelDAG model("sandbox/res/test.bin");
  // RTVE::SparseVoxelDAG model("sandbox/res/testC.bin");
  // RTVE::SparseVoxelDAG model("sandbox/res/highres.bin");

  std::vector<Chunk> chunks;
  // chunks.reserve(10000); // So pointers dont change
  int renderDistance = 10;
  for (int x = -renderDistance; x <= renderDistance; ++x) {
    for (int y = -renderDistance; y <= renderDistance; ++y) {
      if (x*x + y*y < renderDistance)
        chunks.push_back(glm::ivec2(x, y));
    }
  }
  // chunks.emplace_back(glm::ivec2(0, 0));
  for (uint i = 0; i < chunks.size(); ++i)
    camera.attachSparseVoxelDAG(&chunks.at(i).mSVDAG);
  
  camera.mPos = glm::vec3(0, 0, 0);
  camera.setDirection(0, 0);

  glm::vec2 lastMousePos;

  window.mouseCallback = [&camera, &lastMousePos](glm::vec2 pOffset) {
    glm::vec2 offset(pOffset.x - lastMousePos.x, lastMousePos.y - pOffset.y);
    lastMousePos = pOffset;
    
    const float sensitivity = 0.1f;
    offset *= sensitivity;

    camera.setDirection(camera.getYaw() + offset.x, camera.getPitch() + offset.y);
    if (camera.getPitch() > 89.0f)
      camera.setPitch(89.0f);
    if (camera.getPitch() < -89.0f)
      camera.setPitch(-89.0f);

    // std::println("Camera yaw: {}, pitch: {}", camera.getYaw(), camera.getPitch());
    // std::println("Camera direction: {}, {}, {}", camera.getDirectionVector().x, camera.getDirectionVector().y, camera.getDirectionVector().z);
    // std::println("Camera position: {}, {}, {}", camera.mPos.x, camera.mPos.y, camera.mPos.z);
  };
  window.framebufferSizeCallback = [&camera](glm::vec2 pSize) {
    camera.updateViewportSize(pSize);
  };
  // camera.updateViewportSize(window.getSize());

  float deltaTime, lastFrame, lastTimeFPSPrinted = 0.f;
  uint frames = 0;
  bool debugRendering = false;
  bool fLastPressed = false;
  while (!window.shouldWindowClose()) {
    float currentFrame = window.getTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    window.pollEvents();
    window.clear();

    if (debugRendering)
      camera.debugRender(window);
    camera.render();

    // Input - controls
    float cameraSpeed = 10.f * deltaTime;
    if (window.getKeyGLFW(GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
      cameraSpeed *= 10.f;
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

    if (window.getKeyGLFW(GLFW_KEY_F) == GLFW_PRESS && !fLastPressed) {
      fLastPressed = true;
      debugRendering = !debugRendering;
    }
    else if (window.getKeyGLFW(GLFW_KEY_F) == GLFW_RELEASE)
      fLastPressed = false;

    window.swapBuffers();

    // Calculate FPS
    ++frames;
    if (window.getTime() - lastTimeFPSPrinted > 2) {
      std::println("FPS: {}", frames / (window.getTime() - lastTimeFPSPrinted));
      lastTimeFPSPrinted = window.getTime();
      frames = 0;
    }
  }

  // chunk.mSVDAG.releaseDebugMesh();
  skybox.release();
}

