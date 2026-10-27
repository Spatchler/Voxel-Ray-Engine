#include "RTVE.hpp"

#include <unordered_map>

int main(int argc, char* argv[]) {
  if (argc < 3) throw std::invalid_argument("Need to pass <voxel data format(vm8, vm64, vmu, vmc)> <data path with no extension; should be same for palette and voxel data>");
  std::string dataFmt = argv[1];
  if (dataFmt != "vm8" && dataFmt != "vm64" && dataFmt != "vmu" && dataFmt != "vmc") throw std::invalid_argument("Voxel data format arg needs to be one of (vm8, vm64, vmu, vmc)");
  RTVE::Window& window = RTVE::Window::get();
  window.init("RTVE Demo");
  window.captureCursor();
  window.setClearColor(glm::vec4(0.1f, 0.1f, 0.1f, 0.f));

  RTVE::Camera camera;
  camera.updateViewportSize(window.getSize());
  RTVE::Skybox skybox({ RTVE::Path("res/skybox/right.jpeg"),
                        RTVE::Path("res/skybox/left.jpeg"),
                        RTVE::Path("res/skybox/top.jpeg"),
                        RTVE::Path("res/skybox/bottom.jpeg"),
                        RTVE::Path("res/skybox/front.jpeg"),
                        RTVE::Path("res/skybox/back.jpeg")    });
  camera.attachSkybox(&skybox);

  std::string palettePath = argv[2];
  palettePath.append(".pal");
  std::string modelPath = argv[2];
  modelPath.append(".");
  modelPath.append(argv[1]);
  RTVE::ColourPalette palette(palettePath);
  camera.attachColourPalette(&palette);

  window.clear();
  window.swapBuffers();

  RTVE::SparseVoxelDAG model(modelPath);
  camera.reallocateIndicesBuffer(model.mIndices.size() * 8 * sizeof(uint32_t));
  camera.attach(&model);

  VMesh::VoxelGrid* voxelGrid{ nullptr };
  RTVE::SparseVoxelDAG* octree{ nullptr };
  // if (dataFmt == "vm8") {
  //   octree = new RTVE::SparseVoxelDAG(modelPath);
  //   camera.resizeIndicesBuffer(octree->mIndices.size() * 8 * sizeof(uint32_t));
  //   camera.attach(octree);
  // }
  // else if (dataFmt == "vmu") {
  //   voxelGrid = new RTVE::VoxelGrid(modelPath);
  //   camera.attach(voxelGrid);
  // }

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

  skybox.release();

  if (voxelGrid != nullptr) delete voxelGrid;
  if (octree != nullptr) delete octree;
}

