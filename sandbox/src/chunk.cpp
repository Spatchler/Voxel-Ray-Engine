#include "chunk.hpp"

SimplexNoise Chunk::sSimplex = SimplexNoise(0.1f/sScale, 0.5f, sLacunarity, sPersistance);

Chunk::Chunk(glm::vec3 pOffset)
:mSVDAG(0), mOffset(pOffset) {
  std::vector<std::vector<std::vector<bool>>> grid;
  grid = std::vector<std::vector<std::vector<bool>>>(CHUNK_SIZE, std::vector<std::vector<bool>>(CHUNK_SIZE, std::vector<bool>(CHUNK_SIZE, false)));

  for (uint row = 0; row < CHUNK_SIZE; ++row) {
    const float y = static_cast<float>(row + mOffset.y*sScale);

    for (uint col = 0; col < CHUNK_SIZE; ++col) {
      const float x = static_cast<float>(col + mOffset.x*sScale);

      const float noise = std::max(0.f, (sSimplex.fractal(sOctaves, x, y) + mOffset.z) / 2.f);
      uint maxHeight = static_cast<uint>(noise * 100);
      for (uint height = 0; height <= maxHeight; ++height) {
        try {
          grid.at(col).at(height).at(row) = true;
        }
        catch (...) {
          break;
        }
      }
    }
  }
  mSVDAG = RTVE::SparseVoxelDAG(grid);
  mSVDAG.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 0)}); // Air - background color
  mSVDAG.mData.push_back({glm::vec4(1, 1, 1, 0)}); // Block color
}

