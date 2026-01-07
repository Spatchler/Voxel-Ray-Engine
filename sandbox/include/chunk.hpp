#pragma once

#include <SimplexNoise.h>

#include <SparseVoxelDAG.hpp>

#define CHUNK_SIZE 128

class Chunk {
public:
  Chunk(glm::vec3 pOffset);

  RTVE::SparseVoxelDAG mSVDAG;
protected:
  glm::vec3 mOffset;
  static constexpr float sScale    = CHUNK_SIZE;
  static constexpr float sLacunarity    = 1.99f;
  static constexpr float sPersistance   = 0.5f;

  static SimplexNoise sSimplex;
  static const int sOctaves = static_cast<int>(5 + std::log(sScale));
};
