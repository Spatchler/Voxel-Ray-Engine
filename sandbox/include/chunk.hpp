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
  float sScale    = CHUNK_SIZE;
  float sLacunarity    = 1.99f;
  float sPersistance   = 0.5f;
  float sYOffset = 0.5f;

  // SimplexNoise sSimplex;
  SimplexNoise sSimplex = SimplexNoise(0.1f/sScale, 0.5f, sLacunarity, sPersistance);
  const int sOctaves = static_cast<int>(5 + std::log(sScale));
};
