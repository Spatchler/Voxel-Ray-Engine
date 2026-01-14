#pragma once

#include <SimplexNoise.h>

#include "timer.hpp"

#include <cstdlib>
#include <ctime>

#include <SparseVoxelDAG.hpp>

#define CHUNK_SIZE 128

class Chunk {
public:
  Chunk(glm::ivec2 pOffset);

  RTVE::SparseVoxelDAG mSVDAG;

  ~Chunk();
protected:
  void spawnTree(std::vector<std::vector<std::vector<uint32_t>>>& pGrid, glm::ivec3 pPos);

  glm::ivec2 mOffset;
  glm::ivec2 mChunkPos;
  static constexpr float sScale    = CHUNK_SIZE;
  static constexpr float sLacunarity    = 1.99f;
  static constexpr float sPersistance   = 0.5f;
  static constexpr float sYOffset = 1.f;

  static SimplexNoise sSimplex;
  static const int sOctaves = static_cast<int>(5 + std::log(sScale));
};
