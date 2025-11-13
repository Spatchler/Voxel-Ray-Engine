#version 460 core
out vec4 fragColor;
layout(pixel_center_integer) in vec4 gl_FragCoord;

struct VoxelData {
  vec4 color;
};

layout (std430, binding = 0) readonly buffer SVDAGindices {
  uint indices[][8];
};
layout (std430, binding = 1) readonly buffer SVDAGdata {
  VoxelData data[];
};

uniform int uMidpoint;
uniform vec3 uCamPos;
uniform mat4 uProjViewInv;
uniform float uInverseNear;
uniform float uInverseFar;
uniform float uFar;
uniform int uSVOSize;
uniform vec2 uHalfResolutionInv;

vec3 getDirection();
uint toChildIndex(vec3 pPos);
vec3 advanceRay(vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, vec3 pNodeOrigin, uint pNodeSize, inout ivec3 pNormal, inout float pDepth);
uint traverse(vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, inout ivec3 pNormal, inout float pDepth);

void main() {
  vec3 direction = normalize(getDirection());
  vec3 directionInv = 1/direction;
  ivec3 normal = {0, 0, 0};
  float depth = 0;
  uint index = traverse(uCamPos, direction, directionInv, normal, depth);
  gl_FragDepth = ((1/depth) - uInverseNear) / (uInverseFar - uInverseNear);
  VoxelData v = data[index];
  if (index == 0) {
    fragColor = v.color;
    return;
  }
  vec3 light = {0.5, 0.1, 0.76};
  float brightness = (dot(light, normal) + 1) / 2;
  v.color.r = max(0, v.color.r * brightness);
  v.color.g = max(0, v.color.g * brightness);
  v.color.b = max(0, v.color.b * brightness);
  v.color.a = depth;
  fragColor = v.color;
}

vec3 getDirection() {
  vec4 pos = uProjViewInv * vec4(gl_FragCoord.xy * uHalfResolutionInv - 1, 1.f, gl_FragCoord.w);
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

  pos.x = max((pDirection.x * tmin + pOrigin.x) * float(tmin != tx), planeX * float(tmin == tx));
  pos.y = max((pDirection.y * tmin + pOrigin.y) * float(tmin != ty), planeY * float(tmin == ty));
  pos.z = max((pDirection.z * tmin + pOrigin.z) * float(tmin != tz), planeZ * float(tmin == tz));

  pNormal.x = int(sign(pDirectionInv.x)) * -int(pos.x == planeX);
  pNormal.y = int(sign(pDirectionInv.y)) * -int(pos.y == planeY);
  pNormal.z = int(sign(pDirectionInv.z)) * -int(pos.z == planeZ);

  pDepth += tmin;

  return pos;
}

vec3 aabbIntersection(vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, float pMin, float pMax, inout ivec3 pNormal, inout float pDepth) {
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
    // vec3 pos = {pDirection.x * tmin + pOrigin.x, pDirection.y * tmin + pOrigin.y, pDirection.z * tmin + pOrigin.z}; // Calculate intersection
    vec3 pos = {0, 0, 0};

    pos.x = max((pDirection.x * tmin + pOrigin.x) * float(tmin != tx1 && tmin != tx2), max(pMin * float(tmin == tx1), pMax * float(tmin == tx2)));
    pos.y = max((pDirection.y * tmin + pOrigin.y) * float(tmin != ty1 && tmin != ty2), max(pMin * float(tmin == ty1), pMax * float(tmin == ty2)));
    pos.z = max((pDirection.z * tmin + pOrigin.z) * float(tmin != tz1 && tmin != tz2), max(pMin * float(tmin == tz1), pMax * float(tmin == tz2)));

    pos.x = max(0.f, min(uSVOSize, pos.x));
    pos.y = max(0.f, min(uSVOSize, pos.y));
    pos.z = max(0.f, min(uSVOSize, pos.z));

    pNormal.x = -int(pos.x == 0.f);
    pNormal.y = -int(pos.y == 0.f);
    pNormal.z = -int(pos.z == 0.f);

    pDepth = tmin;

    return pos;
  }
  else {
    pDepth = uFar;
    return vec3(-1, -1, -1);
  }
}

uint traverse(vec3 pOrigin, vec3 pDirection, vec3 pDirectionInv, inout ivec3 pNormal, inout float pDepth) {
  if (pOrigin.x > uSVOSize || pOrigin.x < 0 || pOrigin.y > uSVOSize || pOrigin.y < 0 || pOrigin.z > uSVOSize || pOrigin.z < 0) {
    // Ray origin not inside the SVO, carry out aabb intersection
    pOrigin = aabbIntersection(pOrigin, pDirection, pDirectionInv, 0, uSVOSize, pNormal, pDepth);
    if (pOrigin.x < 0) // pOrigin == vec3(-1, -1, -1)
      return 0;
    if ((pOrigin.x == 0 && pDirection.x < 0) ||
        (pOrigin.x == uSVOSize && pDirection.x > 0) ||
        (pOrigin.y == 0 && pDirection.y < 0) ||
        (pOrigin.y == uSVOSize && pDirection.y > 0) ||
        (pOrigin.z == 0 && pDirection.z < 0) ||
        (pOrigin.z == uSVOSize && pDirection.z > 0))
      return 0; // If ray has gone outside the tree return 0
  }

  uint nodeIndex = 0;
  vec3 nodeOrigin = {0, 0, 0};
  uint currentNodeSize = uSVOSize;
  for (;;) {
    // Return if the node is a leaf and is not air
    if (nodeIndex > uMidpoint)
      return nodeIndex - uMidpoint;

    // Advance ray if voxel is air
    if (nodeIndex == uMidpoint) {
      pOrigin = advanceRay(pOrigin, pDirection, pDirectionInv, nodeOrigin, currentNodeSize, pNormal, pDepth);
      if ((pOrigin.x <= 0 && pDirection.x < 0) ||
          (pOrigin.x >= uSVOSize && pDirection.x > 0) ||
          (pOrigin.y <= 0 && pDirection.y < 0) ||
          (pOrigin.y >= uSVOSize && pDirection.y > 0) ||
          (pOrigin.z <= 0 && pDirection.z < 0) ||
          (pOrigin.z >= uSVOSize && pDirection.z > 0)) {
        pDepth = uFar;
        return 0; // If ray has gone outside the tree return 0
      }
      nodeIndex = 0; // Go back to the top of the tree
      nodeOrigin = vec3(0, 0, 0);
      currentNodeSize = uSVOSize;
      continue;
    }

    // Get child at current ray origin if the ray isnt inside a leaf (going deeper in the tree)
    currentNodeSize = currentNodeSize >> 1; // Divide current node size by 2
    vec3 pos = pOrigin;
    pos -= nodeOrigin;
    // pos.x = min(1.f, float(pos.x > currentNodeSize) + min(0.f, sign(pDirectionInv.x)) * float(pos.x == currentNodeSize) );
    // pos.y = min(1.f, float(pos.y > currentNodeSize) + min(0.f, sign(pDirectionInv.y)) * float(pos.y == currentNodeSize) );
    // pos.z = min(1.f, float(pos.z > currentNodeSize) + min(0.f, sign(pDirectionInv.z)) * float(pos.z == currentNodeSize) );
    // nodeOrigin += pos * currentNodeSize;
    pos /= currentNodeSize;
    pos.x = max(0.0, min(1.0, floor(pos.x) + (min(0.f, sign(pDirectionInv.x)) * float(pos.x == 1))));
    pos.y = max(0.0, min(1.0, floor(pos.y) + (min(0.f, sign(pDirectionInv.y)) * float(pos.y == 1))));
    pos.z = max(0.0, min(1.0, floor(pos.z) + (min(0.f, sign(pDirectionInv.z)) * float(pos.z == 1))));
    nodeOrigin += pos * currentNodeSize;
    nodeIndex = indices[nodeIndex][toChildIndex(pos)];
  }
  return 0;
}

