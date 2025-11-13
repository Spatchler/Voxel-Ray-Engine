#include "RTVE.hpp"

#define SVO_SIZE 16

int main() {
  RTVE::Window& window = RTVE::Window::get();
  window.init("RTVE Demo");
  window.captureCursor();

  RTVE::Camera camera;

  RTVE::SparseVoxelDAG world(SVO_SIZE);

  // world.mIndices.push_back({1, 1, 1, 1, 1, 1, 1, 1});
  // world.mIndices.push_back({2, 2, 2, 2, 2, 2, 2, 2});
  // world.mIndices.push_back({3, 3, 3, 3, 3, 3, 3, 3});
  // world.mIndices.push_back({4, 4, 4, 4, 4, 4, 4, 4});
  // world.mIndices.push_back({5, 5, 5, 5, 5, 5, 5, 5});
  // world.mIndices.push_back({6, 6, 6, 6, 6, 6, 6, 6});
  // world.mIndices.push_back({7, 7, 7, 7, 7, 7, 7, 7});
  // world.mIndices.push_back({8, 8, 8, 8, 8, 8, 8, 8});
  // world.mIndices.push_back({9, 9, 9, 9, 9, 9, 9, 9});
  // world.mIndices.push_back({10, 11, 11, 11, 10, 11, 10, 10});
  // world.mData.push_back(RTVE::VoxelData());
  // world.mData.back().color = glm::vec4(0.1, 0.1, 0.1, 1); // Air - background color
  // world.mData.push_back(RTVE::VoxelData());
  // world.mData.back().color = glm::vec4(1, 1, 1, 1);

  world.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 0)}); // Air - background color

  for (int x = -SVO_SIZE; x < SVO_SIZE / 2.f; ++x) {
    for (int y = -SVO_SIZE; y < SVO_SIZE / 2.f; ++y) {
      for (int z = -SVO_SIZE; z < SVO_SIZE / 2.f; ++z) {
        if (std::sqrtf(x*x + z*z + y*y) < SVO_SIZE / 4.f)
          world.insert(glm::vec3(x + SVO_SIZE / 2.f, y + SVO_SIZE / 2.f, z + SVO_SIZE / 2.f), {glm::vec4(1, 1, 1, 0)});
      }
    }
  }
  world.insert(glm::vec3(0, 0, 0), {glm::vec4(1, 1, 1, 0)});
  world.insert(glm::vec3(SVO_SIZE-1, SVO_SIZE-1, SVO_SIZE-1), {glm::vec4(1, 1, 1, 0)});
  world.generateDebugMesh();
  
  camera.mPos = glm::vec3(0, 0, 0);
  camera.setDirection(0, 0);

  camera.attachSparseVoxelDAG(&world);

  glm::vec2 lastMousePos;

  window.mouseCallback = [&camera, &lastMousePos](glm::vec2 pOffset){
    glm::vec2 offset(pOffset.x - lastMousePos.x, lastMousePos.y - pOffset.y);
    lastMousePos = pOffset;
    
    const float sensitivity = 0.1f;
    offset *= sensitivity;

    camera.setDirection(camera.getYaw() + offset.x, camera.getPitch() + offset.y);
    if (camera.getPitch() > 89.0f)
      camera.setPitch(89.0f);
    if (camera.getPitch() < -89.0f)
      camera.setPitch(-89.0f);

    // std::println("Camera direction: {}, {}, {}", camera.getDirectionVector().x, camera.getDirectionVector().y, camera.getDirectionVector().z);
    // std::println("Camera position: {}, {}, {}", camera.mPos.x, camera.mPos.y, camera.mPos.z);
  };

  float deltaTime, lastFrame, lastTimeFPSPrinted = 0.f;;
  uint frames = 0;
  bool debugRendering, fLastPressed = false;
  while (!window.shouldWindowClose()) {
    float currentFrame = window.getTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    window.pollEvents();
    window.clear();

    camera.render(window);
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

  world.releaseDebugMesh();
}

