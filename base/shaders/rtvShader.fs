#version 460 core
out vec4 fragColor;

struct VoxelData {
  bool isAir;
  vec4 color;
};

struct Node {
  bool isLeaf;
  uint childrenIndices[8];
  VoxelData data;
};

layout (std430, binding = 1) readonly buffer SVO {
  Node data[];
};

uniform vec3 camPos;
uniform mat4 viewProjInv;
uniform uint maxDepth;

uint toChildIndex(vec3 pPos) {
  ivec3 localChildPos = {
    int(floor(pPos.x)),
    int(floor(pPos.y)),
    int(floor(pPos.z))
  };
  return (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7
}

uint traverse(vec3 pOrigin, vec3 pDirection) {
  pDirection = normalize(pDirection);

  uint nodeIndex = 0;
  vec3 nodeOrigin = {0, 0, 0};
  for (;;) {
    // Return if the node is a leaf
    if (data[nodeIndex].isLeaf && !data[nodeIndex].data.isAir)
      return nodeIndex;

    // Get child at ray origin
    // vec3 localRayOrigin = pOrigin - nodeOrigin;
    // uint globalChildIndex = max(data[nodeIndex].childrenIndices[toChildIndex(localRayOrigin)] - 1, 0); // Index in data array
    // Return if ray starts in a leaf
    // if (data[globalChildIndex].isLeaf && !data[globalChildIndex].data.isAir)
      // return globalChildIndex;

    if (data[nodeIndex].isLeaf) {
      // Progress ray to next intersection
      float planeX = -1;
      float planeY = -1;
      float planeZ = -1;

      // X
      if (pDirection.x > 0) {
        planeX = nodeOrigin.x + 1;
        t = (planeX - pOrigin.x) / pDirection.x;
      }
      else if (pDirection.x < 0) {
        planeX = nodeOrigin.x;
        t = (planeX - pOrigin.x) / pDirection.x;
      }

      // Y
      if (pDirection.y > 0) {
        planeY = nodeOrigin.y + 1;
        float tempT = (planeY - pOrigin.y) / pDirection.y;
        if (tempT < t) {
          t = tempT;
          nodeIndex = max(data[nodeIndex].childrenIndices[toChildIndex(vec3(nodeChildPos, pDirection.y, ))] - 1, 0); // Index in data array
          continue;
        }
      }
      else if (pDirection.y < 0) {
        planeY = nodeOrigin.y;
        t = (planeY - pOrigin.y) / pDirection.y;
      }

      // Z
      if (pDirection.z > 0) {
        planeZ = nodeOrigin.z + 1;
      }
      else if (pDirection.z < 0) {
        planeZ = nodeOrigin.z;
      }

      if (planeX != -1) {
        t = (planeX - pOrigin.x) / pDirection.x;
      }
      if (planeY != -1) {
        t = (planeY - pOrigin.y) / pDirection.y;
      }
      if (planeZ != -1) {
        t = (planeZ - pOrigin.z) / pDirection.z;
      }
    }
    else {
      // Go deeper in the tree
    }

    // Get child at ray origin
    vec3 localRayOrigin = pOrigin - nodeOrigin;
    ivec3 localChildPos = {
      int(floor(localRayOrigin.x)),
      int(floor(localRayOrigin.y)),
      int(floor(localRayOrigin.z))
    };
    uint localChildIndex = (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7

    uint globalChildIndex = max(data[nodeIndex].childrenIndices[localChildIndex] - 1, 0); // Index in data array
    // Return if ray starts in a leaf
    if (data[globalChildIndex].isLeaf)
      return globalChildIndex;

    // Move to next intersection
    vec3 intersectionDirection = {0, 0, 0};
    float t = 100;
    if (pDirection.x > 0) {
      float x = localChildPos.x + 1;
      t = min((x - localRayOrigin.x) / pDirection.x, t);
    }
    else {
      float x = localChildPos.x - 1;
      t = min((x - localRayOrigin.x) / pDirection.x, t);
    }

    if (pDirection.y > 0) {
      float y = localChildPos.y + 1;
      t = min((y - localRayOrigin.y) / pDirection.y, t);
    }
    else {
      float y = localChildPos.y - 1;
      t = min((y - localRayOrigin.y) / pDirection.y, t);
    }

    if (pDirection.z > 0) {
      float z = localChildPos.z + 1;
      t = min((z - localRayOrigin.z) / pDirection.z, t);
    }
    else {
      float z = localChildPos.z - 1;
      t = min((z - localRayOrigin.z) / pDirection.z, t);
    }
    vec3 localIntersection= {pDirection.x * t + localRayOrigin.x, pDirection.y * t + localRayOrigin.y, pDirection.z * t + localRayOrigin.z};
    localChildPos = ivec3(
      int(floor(localIntersection.x)),
      int(floor(localIntersection.y)),
      int(floor(localIntersection.z))
    );
    localChildIndex = (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7
    globalChildIndex = max(data[nodeIndex].childrenIndices[localChildIndex] - 1, 0); // Index in data array
    if (data[globalChildIndex].isLeaf)
      return globalChildIndex;

    nodeIndex = localChildIndex;
    pOrigin = nodeOrigin + localIntersection;
    nodeOrigin += localChildPos;
  }

  return 0;
}

vec3 getDirection() {
  vec4 pos = gl_FragCoord * viewProjInv;
  return pos.xyz / pos.w;
}

void main() {
  VoxelData v = data[traverse(camPos, getDirection())].data;
  fragColor = v.color;
}

