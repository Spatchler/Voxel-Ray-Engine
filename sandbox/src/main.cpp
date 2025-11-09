#include "RTVE.hpp"

#define SVO_SIZE 128

glm::vec3 aabbIntersection(glm::vec3 pOrigin, glm::vec3 pDirection, glm::vec3 pDirectionInv, float pMin, float pMax, glm::vec3& pNormal) {
  float tx1 = (pMin - pOrigin.x) * pDirectionInv.x;
  float tx2 = (pMax - pOrigin.x) * pDirectionInv.x;

  float tmin = std::min(tx1, tx2);
  float tmax = std::max(tx1, tx2);

  float ty1 = (pMin - pOrigin.y) * pDirectionInv.y;
  float ty2 = (pMax - pOrigin.y) * pDirectionInv.y;

  tmin = std::max(tmin, std::min(ty1, ty2));
  tmax = std::min(tmax, std::max(ty1, ty2));

  float tz1 = (pMin - pOrigin.z) * pDirectionInv.z;
  float tz2 = (pMax - pOrigin.z) * pDirectionInv.z;

  tmin = std::max(tmin, std::min(tz1, tz2));
  tmax = std::min(tmax, std::max(tz1, tz2));

  if (tmin >= 0 && tmax >= tmin) {
    glm::vec3 pos(0, 0, 0);

    pos.x = std::max((pDirection.x * tmin + pOrigin.x) * (tmin != tx1 && tmin != tx2), std::max(pMin * (tmin == tx1), pMax * (tmin == tx2)));
    pos.y = std::max((pDirection.y * tmin + pOrigin.y) * (tmin != ty1 && tmin != ty2), std::max(pMin * (tmin == ty1), pMax * (tmin == ty2)));
    pos.z = std::max((pDirection.z * tmin + pOrigin.z) * (tmin != tz1 && tmin != tz2), std::max(pMin * (tmin == tz1), pMax * (tmin == tz2)));

    pos.x = std::max(0.f, std::min((float)SVO_SIZE, pos.x));
    pos.y = std::max(0.f, std::min((float)SVO_SIZE, pos.y));
    pos.z = std::max(0.f, std::min((float)SVO_SIZE, pos.z));

    pNormal.x = pos.x == 0.f;
    pNormal.y = pos.y == 0.f;
    pNormal.z = pos.z == 0.f;

    return pos;
  }
  else
    return glm::vec3(-1, -1, -1);
}

uint toChildIndex(glm::vec3 pPos) {
  glm::tvec3<int, glm::packed_highp> localChildPos = {
    int(floor(pPos.x)),
    int(floor(pPos.y)),
    int(floor(pPos.z))
  };
  return (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7
}

glm::vec3 advanceRay(glm::vec3 pOrigin, glm::vec3 pDirection, glm::vec3 pDirectionInv, glm::vec3 pNodeOrigin, uint pNodeSize, glm::vec3& pNormal) {
  float planeX = pNodeOrigin.x + pNodeSize*std::max(0.f, glm::sign(pDirectionInv.x));
  float tx = (planeX - pOrigin.x) * pDirectionInv.x;

  float planeY = pNodeOrigin.y + pNodeSize*std::max(0.f, glm::sign(pDirectionInv.y));
  float ty = (planeY - pOrigin.y) * pDirectionInv.y;

  float planeZ = pNodeOrigin.z + pNodeSize*std::max(0.f, glm::sign(pDirectionInv.z));
  float tz = (planeZ - pOrigin.z) * pDirectionInv.z;

  float tmin = std::min(tx, std::min(ty, tz));

  glm::vec3 pos(0, 0, 0);

  pos.x = std::max((pDirection.x * tmin + pOrigin.x) * (tmin != tx), planeX * (tmin == tx));
  pos.y = std::max((pDirection.y * tmin + pOrigin.y) * (tmin != ty), planeY * (tmin == ty));
  pos.z = std::max((pDirection.z * tmin + pOrigin.z) * (tmin != tz), planeZ * (tmin == tz));

  pNormal.x = glm::sign(pDirectionInv.x) * (pos.x == planeX);
  pNormal.y = glm::sign(pDirectionInv.y) * (pos.y == planeY);
  pNormal.z = glm::sign(pDirectionInv.z) * (pos.z == planeZ);

  return pos;
}

uint traverse(RTVE::SparseVoxelDAG& pSVO, glm::vec3 pOrigin, glm::vec3 pDirection, glm::vec3& pNormal) {
  pDirection = normalize(pDirection);
  glm::vec3 directionInv = 1.f/pDirection;

  if (pOrigin.x > SVO_SIZE || pOrigin.x < 0 || pOrigin.y > SVO_SIZE || pOrigin.y < 0 || pOrigin.z > SVO_SIZE || pOrigin.z < 0) {
    // Ray origin not inside the SVO, carry out aabb intersection
    pOrigin = aabbIntersection(pOrigin, pDirection, directionInv, 0, SVO_SIZE, pNormal);
    if (pOrigin == glm::vec3(-1, -1, -1))
      return 0;
    if ((pOrigin.x <= 0 && pDirection.x < 0) ||
        (pOrigin.x >= SVO_SIZE && pDirection.x > 0) ||
        (pOrigin.y <= 0 && pDirection.y < 0) ||
        (pOrigin.y >= SVO_SIZE && pDirection.y > 0) ||
        (pOrigin.z <= 0 && pDirection.z < 0) ||
        (pOrigin.z >= SVO_SIZE && pDirection.z > 0))
      return 0; // If ray has gone outside the tree return 0
  }

  uint nodeIndex = 0;
  glm::vec3 nodeOrigin = {0, 0, 0};
  uint currentNodeSize = SVO_SIZE;
  for (;;) {
    // Return if the node is a leaf and is not air
    if (nodeIndex > pSVO.getMidpoint())
      return nodeIndex - pSVO.getMidpoint();

    // Advance ray if voxel is air
    if (nodeIndex == pSVO.getMidpoint()) {
      pOrigin = advanceRay(pOrigin, pDirection, directionInv, nodeOrigin, currentNodeSize, pNormal);
      if ((pOrigin.x <= 0 && pDirection.x < 0) ||
          (pOrigin.x >= SVO_SIZE && pDirection.x > 0) ||
          (pOrigin.y <= 0 && pDirection.y < 0) ||
          (pOrigin.y >= SVO_SIZE && pDirection.y > 0) ||
          (pOrigin.z <= 0 && pDirection.z < 0) ||
          (pOrigin.z >= SVO_SIZE && pDirection.z > 0))
        return 0; // If ray has gone outside the tree return 0
      nodeIndex = 0; // Go back to the top of the tree
      nodeOrigin = glm::vec3(0, 0, 0);
      currentNodeSize = SVO_SIZE;
      continue;
    }

    // Get child at current ray origin if the ray isnt inside a leaf (going deeper in the tree)
    currentNodeSize = currentNodeSize >> 1; // Divide current node size by 2
    glm::vec3 pos = pOrigin;
    pos -= nodeOrigin;
    pos /= currentNodeSize;
    pos.x = std::max(0.0, std::min(1.0, floor(pos.x) + (std::min(0.f, glm::sign(directionInv.x)) * (floor(pos.x) == 1))));
    pos.y = std::max(0.0, std::min(1.0, floor(pos.y) + (std::min(0.f, glm::sign(directionInv.y)) * (floor(pos.y) == 1))));
    pos.z = std::max(0.0, std::min(1.0, floor(pos.z) + (std::min(0.f, glm::sign(directionInv.z)) * (floor(pos.z) == 1))));
    nodeOrigin += pos * (float)currentNodeSize;
    nodeIndex = pSVO.mIndices[nodeIndex][toChildIndex(pos)];
  }
  return 0;
}

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

  world.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 1)}); // Air - background color
  world.insert(glm::vec3(0, 0, 0), {glm::vec4(1, 1, 1, 1)});
  world.print();
  world.insert(glm::vec3(10, 0, 0), {glm::vec4(10, 1, 1, 1)});
  world.print();
  
  camera.mPos = glm::vec3(0, 0, 0);
  camera.setDirection(0, 0);
  std::println("Camera direction: {}, {}, {}", camera.getDirectionVector().x, camera.getDirectionVector().y, camera.getDirectionVector().z);

  std::println("First CPU traversal test (Normal) --------------------------------------");
  glm::vec3 normal;
  glm::vec4 c = world.mData[traverse(world, glm::vec3(0, 0, 0), glm::vec3(1, 0, 0), normal)].color;
  std::println("Normal: {}, {}, {}", normal.x, normal.y, normal.z);
  std::println("{}, {}, {}, {}", c.x, c.y, c.z, c.w);
  std::println("First test complete");

  std::println("Second CPU traversal test (Corner) -------------------------------------");
  c = world.mData[traverse(world, glm::vec3(-1, 1, 0), glm::vec3(1, -1, 0), normal)].color;
  std::println("Normal: {}, {}, {}", normal.x, normal.y, normal.z);
  std::println("{}, {}, {}, {}", c.x, c.y, c.z, c.w);
  std::println("Second test complete");

  std::println("Third CPU traversal test (Behind) --------------------------------------");
  c = world.mData[traverse(world, glm::vec3(17, 0, 17), glm::vec3(-1, 0, -1), normal)].color;
  std::println("Normal: {}, {}, {}", normal.x, normal.y, normal.z);
  std::println("{}, {}, {}, {}", c.x, c.y, c.z, c.w);
  std::println("Third test complete");

  std::println("Fourth CPU traversal test (General) ------------------------------------");
  c = world.mData[traverse(world, glm::vec3(3.7, 3.6, 2.4), glm::vec3(0.6, 0.8, 0), normal)].color;
  std::println("Normal: {}, {}, {}", normal.x, normal.y, normal.z);
  std::println("{}, {}, {}, {}", c.x, c.y, c.z, c.w);
  std::println("Fourth test complete");

  std::println("Fifth CPU traversal test (General) ------------------------------------");
  c = world.mData[traverse(world, glm::vec3(0, 0, 0), glm::vec3(1, 1, 1), normal)].color;
  std::println("Normal: {}, {}, {}", normal.x, normal.y, normal.z);
  std::println("{}, {}, {}, {}", c.x, c.y, c.z, c.w);
  std::println("Fifth test complete");

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
  while (!window.shouldWindowClose()) {
    float currentFrame = window.getTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    window.pollEvents();
    window.clear();

    camera.render(window);

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

    window.swapBuffers();

    // Calculate FPS
    ++frames;
    if (window.getTime() - lastTimeFPSPrinted > 2) {
      std::println("FPS: {}", frames / (window.getTime() - lastTimeFPSPrinted));
      lastTimeFPSPrinted = window.getTime();
      frames = 0;
    }
  }
}

