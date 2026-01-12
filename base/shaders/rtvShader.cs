#version 460 core
// layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout (local_size_x = 30, local_size_y = 30, local_size_z = 1) in;
layout (rgba32f, binding = 0) uniform image2D imgOut;

struct VoxelData {
  vec4 color;
};

struct Node {
  vec3 origin;
  uint index;
};

struct Metadata {
  uint indicesIndex;
  uint midpoint;
  uint size;
  vec3 translation;
};

layout (std430, binding = 0) readonly buffer SVDAGindicesSSBO {
  uint bIndices[][8];
};
layout (std430, binding = 1) readonly buffer SVDAGdataSSBO {
  VoxelData bData[];
};
layout (std430, binding = 2) readonly buffer MetadataSSBO {
  Metadata bMetadata[];
};

uniform vec3 uCamPos;
uniform mat4 uProjViewInv;
uniform float uInverseNear;
uniform float uInverseFrustumDepth;
uniform float uFar;
uniform vec2 uHalfResolutionInv;
uniform uint uNumOctrees;

vec3 getDirection();
uint toChildIndex(vec3 pPos);
vec3 advanceRay(vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, vec3 pNodeOrigin, uint pNodeSize, inout ivec3 pNormal, inout float pDepth);
uint traverse(uint pRootIndex, uint pMidpoint, uint pSize, vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, inout ivec3 pNormal, inout float pDepth, inout float pAdvanceCount);

void main() {
  vec3 direction = normalize(getDirection());
  vec3 directionInv = 1/direction;
  ivec3 normal = {0, 0, 0};
  float depth = 0;
  float advanceCount = 0;

  VoxelData v = bData[0];
  v.color.a = ((1/uFar) - uInverseNear) * uInverseFrustumDepth;
  imageStore(imgOut, ivec2(gl_GlobalInvocationID.xy), v.color);

  vec3 light = {0.5, 0.1, 0.76};

  for (uint i = 0; i < uNumOctrees; ++i) {
    vec3 camPos = uCamPos;
    camPos -= bMetadata[i].translation.xyz;

    uint index = traverse(bMetadata[i].indicesIndex, bMetadata[i].midpoint, bMetadata[i].size, camPos, direction, directionInv, normal, depth, advanceCount);

    if (index != 0) {
      v = bData[index];

      v.color.a = ((1/depth) - uInverseNear) * uInverseFrustumDepth;
      if (v.color.a > imageLoad(imgOut, ivec2(gl_GlobalInvocationID.xy)).a)
        continue;
      
      float brightness = (dot(light, normal) + 1) / 2;

      v.color.r = max(0, v.color.r * brightness);
      v.color.g = max(0, v.color.g * brightness);
      v.color.b = max(0, v.color.b * brightness);

      imageStore(imgOut, ivec2(gl_GlobalInvocationID.xy), v.color);
    }
  }
}

vec3 getDirection() {
  vec4 pos = uProjViewInv * vec4(gl_GlobalInvocationID.xy * uHalfResolutionInv - 1, 1.f, 1.f);
  // return pos.xyz / pos.w;
  return pos.xyz;
}

uint toChildIndex(vec3 pPos) {
  ivec3 localChildPos = {
    int(floor(pPos.x)),
    int(floor(pPos.y)),
    int(floor(pPos.z))
  };
  return (localChildPos.x << 0) | (localChildPos.y << 1) | (localChildPos.z << 2); // Index in childIndices 0 - 7
}

vec3 advanceRay(vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, vec3 pNodeOrigin, uint pNodeSize, inout ivec3 pNormal, inout float pDepth) {
  float planeX = pNodeOrigin.x + pNodeSize*max(0.f, sign(pDirectionInv.x));
  float tx = (planeX - pOrigin.x) * pDirectionInv.x;

  float planeY = pNodeOrigin.y + pNodeSize*max(0.f, sign(pDirectionInv.y));
  float ty = (planeY - pOrigin.y) * pDirectionInv.y;

  float planeZ = pNodeOrigin.z + pNodeSize*max(0.f, sign(pDirectionInv.z));
  float tz = (planeZ - pOrigin.z) * pDirectionInv.z;

  float tmin = min(tx, min(ty, tz));

  vec3 pos = {0, 0, 0};

  if (tmin == tx)
    pos.x = planeX;
  else
    pos.x = pDirection.x * tmin + pOrigin.x;

  if (tmin == ty)
    pos.y = planeY;
  else
    pos.y = pDirection.y * tmin + pOrigin.y;

  if (tmin == tz)
    pos.z = planeZ;
  else
    pos.z = pDirection.z * tmin + pOrigin.z;

  pNormal.x = int(sign(pDirectionInv.x)) * -int(pos.x == planeX);
  pNormal.y = int(sign(pDirectionInv.y)) * -int(pos.y == planeY);
  pNormal.z = int(sign(pDirectionInv.z)) * -int(pos.z == planeZ);

  pDepth += tmin;

  return pos;
}

vec3 aabbIntersection(uint pSize, vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, float pMin, float pMax, inout ivec3 pNormal, inout float pDepth) {
  float tx1 = (pMin - pOrigin.x) * pDirectionInv.x;
  float tx2 = (pMax - pOrigin.x) * pDirectionInv.x;

  float tmin = min(tx1, tx2);
  float tmax = max(tx1, tx2);

  float ty1 = (pMin - pOrigin.y) * pDirectionInv.y;
  float ty2 = (pMax - pOrigin.y) * pDirectionInv.y;

  tmin = max(tmin, min(ty1, ty2));
  tmax = min(tmax, max(ty1, ty2));

  float tz1 = (pMin - pOrigin.z) * pDirectionInv.z;
  float tz2 = (pMax - pOrigin.z) * pDirectionInv.z;

  tmin = max(tmin, min(tz1, tz2));
  tmax = min(tmax, max(tz1, tz2));

  if (tmin >= 0 && tmax >= tmin) {
    vec3 pos = {pDirection.x * tmin + pOrigin.x, pDirection.y * tmin + pOrigin.y, pDirection.z * tmin + pOrigin.z}; // Calculate intersection
    // vec3 pos = {0, 0, 0};

    // if (tmin != tx1 && tmin != tx1)
    //   pos.x = pDirection.x * tmin + pOrigin.x;
    // else if (tmin == tx1)
    //   pos.x = pMin;
    // else
    //   pos.x = pMax;

    // if (tmin != ty1 && tmin != ty1)
    //   pos.y = pDirection.y * tmin + pOrigin.y;
    // else if (tmin == ty1)
    //   pos.y = pMin;
    // else
    //   pos.y = pMax;

    // if (tmin != tz1 && tmin != tz1)
    //   pos.z = pDirection.z * tmin + pOrigin.z;
    // else if (tmin == tz1)
    //   pos.z = pMin;
    // else
    //   pos.z = pMax;

    pos.x = max(0.f, min(pSize, pos.x));
    pos.y = max(0.f, min(pSize, pos.y));
    pos.z = max(0.f, min(pSize, pos.z));

    pNormal.x = -int(tmin == tx1) + int(tmin == tx2);
    pNormal.y = -int(tmin == ty1) + int(tmin == ty2);
    pNormal.z = -int(tmin == tz1) + int(tmin == tz2);
    // pNormal.x = int(sign(pDirectionInv.x)) * -int(pos.x == 0.f);
    // pNormal.y = int(sign(pDirectionInv.y)) * -int(pos.y == 0.f);
    // pNormal.z = int(sign(pDirectionInv.z)) * -int(pos.z == 0.f);

    pDepth = tmin;

    return pos;
  }
  else {
    pDepth = uFar;
    return vec3(-1, -1, -1);
  }
}

uint traverse(uint pRootIndex, uint pMidpoint, uint pSize, vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, inout ivec3 pNormal, inout float pDepth, inout float pAdvanceCount) {
  if (pOrigin.x > pSize || pOrigin.x < 0 || pOrigin.y > pSize || pOrigin.y < 0 || pOrigin.z > pSize || pOrigin.z < 0) {
    // Ray origin not inside the grid, carry out aabb intersection
    pOrigin = aabbIntersection(pSize, pOrigin, pDirection, pDirectionInv, 0, pSize, pNormal, pDepth);
    if (pOrigin.x < 0) // pOrigin == vec3(-1, -1, -1)
      return 0;
    if ((pOrigin.x == 0 && pDirection.x < 0) ||
        (pOrigin.x == pSize && pDirection.x > 0) ||
        (pOrigin.y == 0 && pDirection.y < 0) ||
        (pOrigin.y == pSize && pDirection.y > 0) ||
        (pOrigin.z == 0 && pDirection.z < 0) ||
        (pOrigin.z == pSize && pDirection.z > 0))
      return 0; // If ray has gone outside the tree return 0
  }

  Node currentNode;
  currentNode.origin = vec3(0, 0, 0);
  currentNode.index = pRootIndex;
  uint currentNodeSize = pSize;
  uint depth = 0;
  for (;;) {
    // Return if the node is a leaf and is not air
    if (currentNode.index - pRootIndex > pMidpoint)
      return currentNode.index - pRootIndex - pMidpoint;

    // Advance ray if voxel is air
    if (currentNode.index - pRootIndex == pMidpoint) {
      // ++pAdvanceCount;
      pOrigin = advanceRay(pOrigin, pDirection, pDirectionInv, currentNode.origin, currentNodeSize, pNormal, pDepth);
      if ((pOrigin.x <= 0 && pDirection.x < 0) ||
          (pOrigin.x >= pSize && pDirection.x > 0) ||
          (pOrigin.y <= 0 && pDirection.y < 0) ||
          (pOrigin.y >= pSize && pDirection.y > 0) ||
          (pOrigin.z <= 0 && pDirection.z < 0) ||
          (pOrigin.z >= pSize && pDirection.z > 0)) {
        pDepth = uFar;
        return 0; // If ray has gone outside the tree return 0
      }
      depth = 0;
      currentNode.index = pRootIndex;
      currentNode.origin = vec3(0, 0, 0);
      currentNodeSize = pSize;
      continue;
    }

    // Get child at current ray origin if the ray isnt inside a leaf (going deeper in the tree)
    ++pAdvanceCount;
    currentNodeSize = currentNodeSize >> 1; // Divide current node size by 2
    vec3 pos = pOrigin;
    pos -= currentNode.origin;
    // pos.x = min(1.f, float(pos.x > currentNodeSize) + min(0.f, sign(pDirectionInv.x)) * float(pos.x == currentNodeSize) );
    // pos.y = min(1.f, float(pos.y > currentNodeSize) + min(0.f, sign(pDirectionInv.y)) * float(pos.y == currentNodeSize) );
    // pos.z = min(1.f, float(pos.z > currentNodeSize) + min(0.f, sign(pDirectionInv.z)) * float(pos.z == currentNodeSize) );
    // nodeOrigin += pos * currentNodeSize;
    pos /= currentNodeSize;
    if (pos.x == 1)
      pos.x = max(0.0, min(1.0, floor(pos.x) + min(0.f, sign(pDirectionInv.x))));
    else
      pos.x = max(0.0, min(1.0, floor(pos.x)));

    if (pos.y == 1)
      pos.y = max(0.0, min(1.0, floor(pos.y) + min(0.f, sign(pDirectionInv.y))));
    else
      pos.y = max(0.0, min(1.0, floor(pos.y)));

    if (pos.z == 1)
      pos.z = max(0.0, min(1.0, floor(pos.z) + min(0.f, sign(pDirectionInv.z))));
    else
      pos.z = max(0.0, min(1.0, floor(pos.z)));
    // pos.x = max(0.0, min(1.0, floor(pos.x) + (min(0.f, sign(pDirectionInv.x)) * float(pos.x == 1))));
    // pos.y = max(0.0, min(1.0, floor(pos.y) + (min(0.f, sign(pDirectionInv.y)) * float(pos.y == 1))));
    // pos.z = max(0.0, min(1.0, floor(pos.z) + (min(0.f, sign(pDirectionInv.z)) * float(pos.z == 1))));
    Node n;
    n.origin = currentNode.origin + (pos * currentNodeSize);
    n.index = pRootIndex + bIndices[currentNode.index][toChildIndex(pos)];
    currentNode = n;
    ++depth;
  }
  return 0;
}

