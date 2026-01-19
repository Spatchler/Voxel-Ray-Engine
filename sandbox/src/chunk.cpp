#include "chunk.hpp"

// SimplexNoise Chunk::sSimplex = SimplexNoise(0.1f/sScale, 0.5f, sLacunarity, sPersistance);

Chunk::Chunk(glm::ivec2 pOffset)
:mSVDAG(0), mChunkPos(pOffset), mOffset(pOffset * CHUNK_SIZE) {
  RTVE::ScopedTimer t("Generating chunk");
  std::vector<std::vector<std::vector<uint32_t>>> grid;
  grid = std::vector<std::vector<std::vector<uint32_t>>>(CHUNK_SIZE, std::vector<std::vector<uint32_t>>(CHUNK_SIZE, std::vector<uint32_t>(CHUNK_SIZE, 0)));

  srand(time(0));

  for (uint row = 0; row < CHUNK_SIZE; ++row) {
    const float y = static_cast<float>(row) + mOffset.y;

    for (uint col = 0; col < CHUNK_SIZE; ++col) {
      const float x = static_cast<float>(col) + mOffset.x;

      const float noise = std::max(0.f, (sSimplex.fractal(sOctaves, x, y) + sYOffset) / 2.f);
      uint maxHeight = std::max(1.f, std::min(CHUNK_SIZE - 1.f, noise * 200));
      for (uint height = 0; height <= maxHeight; ++height) {
        try {
          if (maxHeight <= 1)
            grid.at(col).at(height).at(row) = 3;
          else if (height == maxHeight)
            grid.at(col).at(height).at(row) = 2;
          else
            grid.at(col).at(height).at(row) = 1;
        }
        catch (...) {
          std::println("Failed to insert voxel");
          break;
        }
      }
      if (maxHeight > 1) {
        int random = rand() % 41;
        if (random == 1)
          spawnTree(grid, glm::ivec3(col, maxHeight, row));
      }
    }
  }
  mSVDAG = RTVE::SparseVoxelDAG(grid);
  mSVDAG.translate(glm::vec3(mOffset.x, 0, mOffset.y));

  // mSVDAG.generateDebugMesh();
}

Chunk::~Chunk() {
  // mSVDAG.releaseDebugMesh();
}

void Chunk::spawnTree(std::vector<std::vector<std::vector<uint32_t>>>& pGrid, glm::ivec3 pPos) {
  try {
    pGrid.at(pPos.x).at(pPos.y).at(pPos.z) = 4;
    pGrid.at(pPos.x).at(pPos.y + 1).at(pPos.z) = 4;
    pGrid.at(pPos.x).at(pPos.y + 2).at(pPos.z) = 4;
    pGrid.at(pPos.x).at(pPos.y + 3).at(pPos.z) = 4;
    pGrid.at(pPos.x).at(pPos.y + 4).at(pPos.z) = 4;
  }
  catch (...) {
    std::println("Failed to insert voxel");
    return;
  }
}

