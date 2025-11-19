#include "RTVE.hpp"

#define SVDAG_SIZE 64

int main() {
  RTVE::Window& window = RTVE::Window::get();
  window.init("RTVE Demo");
  window.captureCursor();
  window.setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.f));

  RTVE::Camera camera;

  // RTVE::SparseVoxelDAG world(SVDAG_SIZE);

  // world.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 1)}); // Air - background color

  // for (int x = -SVDAG_SIZE; x < SVDAG_SIZE / 2.f; ++x) {
  //   for (int y = -SVDAG_SIZE; y < SVDAG_SIZE / 2.f; ++y) {
  //     for (int z = -SVDAG_SIZE; z < SVDAG_SIZE / 2.f; ++z) {
  //       if (std::sqrtf(x*x + z*z + y*y) < SVDAG_SIZE / 2.f)
  //         world.insert(glm::vec3(x + SVDAG_SIZE / 2.f, y + SVDAG_SIZE / 2.f, z + SVDAG_SIZE / 2.f), {glm::vec4(1, 1, 1, 0)});
  //     }
  //   }
  // }
  // world.generateDebugMesh();

  RTVE::SVDAGModel model("sandbox/res/HollowKnight.obj", SVDAG_SIZE);
  // model.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 1)}); // Air - background color
  model.generateDebugMesh();
  camera.attachSparseVoxelDAG(&model);
  
  camera.mPos = glm::vec3(0, 0, 0);
  camera.setDirection(0, 0);
  // camera.mPos = glm::vec3(SVDAG_SIZE * 2, SVDAG_SIZE * 2, SVDAG_SIZE * 2);
  // camera.setDirection(-135.f, -45.f);

  // camera.attachSparseVoxelDAG(&world);

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
  camera.updateViewportSize(window.getSize());

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

    camera.render();
    if (debugRendering)
      camera.debugRender(window);

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

  // world.releaseDebugMesh();
}

