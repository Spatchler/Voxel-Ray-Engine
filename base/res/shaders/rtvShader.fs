#version 460 core
out vec4 fragColor;

struct VoxelData {
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
uniform mat4 projViewInv;
uniform uint maxDepth;

// Returns index of the voxel that the ray hits first
uint traverseImpl(uint pNodeIndex, vec3 pOrigin, vec3 pDirection, vec3 pNodeOrigin) {
  // Return if the node is a leaf
  if (data[pNodeIndex].isLeaf)
    return pNodeIndex;

  // Get child at ray origin
  vec3 localRayOrigin = pOrigin - pNodeOrigin;
  ivec3 localChildPos = {
    int(floor(localRayOrigin.x)),
    int(floor(localRayOrigin.y)),
    int(floor(localRayOrigin.z))
  };
  uint localChildIndex = (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7

  uint globalChildIndex = data[pNodeIndex].childrenIndices[localChildIndex]; // Index in data array
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
  globalChildIndex = data[pNodeIndex].childrenIndices[localChildIndex]; // Index in data array
  if (data[globalChildIndex].isLeaf)
    return globalChildIndex;

  return traverseImpl(localChildIndex, pNodeOrigin + localIntersection, pDirection, pNodeOrigin + localChildPos);
}

uint traverse(vec3 pOrigin, vec3 pDirection) {
  pDirection = normalize(pDirection);
  // return traverseImpl(0, pOrigin, normalize(pDirection), vec3(0, 0, 0));

  uint nodeIndex = 0;
  vec3 nodeOrigin = {0, 0, 0};
  for (;;) {
    // Return if the node is a leaf
    if (data[nodeIndex].isLeaf)
      return nodeIndex;

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
  vec4 pos = gl_FragCoord * projViewInv;
  return pos.xyz / pos.w;
}

void main() {
  VoxelData v = data[traverse(camPos, getDirection())].data;
  fragColor = v.color;
}

