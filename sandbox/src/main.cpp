#include "RTVE.hpp"

int main() {
  RTVE::Window& window = RTVE::Window::get();
  window.init("RTVE Demo");

  RTVE::Camera camera;

  RTVE::SparseVoxelOctree world(10, 1);

  world.mNodes[0].isLeaf = true;
  world.mNodes[0].data.color = glm::vec4(1, 0, 0, 1);

  camera.attachSVO(&world);

  while (!window.shouldWindowClose()) {
    window.pollEvents();
    window.clear();

    camera.render(window);

    window.swapBuffers();
  }
}

