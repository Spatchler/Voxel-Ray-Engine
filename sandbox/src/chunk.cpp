#include "chunk.hpp"

SimplexNoise Chunk::sSimplex = SimplexNoise(0.1f/sScale, 0.5f, sLacunarity, sPersistance);

Chunk::Chunk(glm::ivec2 pOffset)
:mSVDAG(0), mChunkPos(pOffset), mOffset(pOffset * CHUNK_SIZE) {
  RTVE::ScopedTimer t("Generating chunk");
  std::vector<std::vector<std::vector<uint32_t>>> grid;
  grid = std::vector<std::vector<std::vector<uint32_t>>>(CHUNK_SIZE, std::vector<std::vector<uint32_t>>(CHUNK_SIZE, std::vector<uint32_t>(CHUNK_SIZE, 0)));

  // mSVDAG = RTVE::SparseVoxelDAG(CHUNK_SIZE);
  // mSVDAG.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 0)}); // Air - background color

  // mSVDAG.insert(glm::vec3(10, 10, 10), {glm::vec4(0.07, 0.63, 0.18, 0)});

  srand(time(0));

  for (uint row = 0; row < CHUNK_SIZE; ++row) {
    const float y = static_cast<float>(row) + mOffset.y;

    for (uint col = 0; col < CHUNK_SIZE; ++col) {
      const float x = static_cast<float>(col) + mOffset.x;

      const float noise = std::max(0.f, (sSimplex.fractal(sOctaves, x, y) + sYOffset) / 2.f);
      uint maxHeight = static_cast<uint>(noise * 100);
      if (maxHeight < 50)
        maxHeight = 50;
      for (uint height = 0; height <= maxHeight; ++height) {
        try {
          // grid.at(col).at(height).at(row) = 1;
          // std::println("insert pos: {}, {}, {}", col, height, row);
          if (height <= 50)
            grid.at(col).at(height).at(row) = 2;
          else
            grid.at(col).at(height).at(row) = 1;
        }
        catch (...) {
          std::println("Failed to insert voxel");
          break;
        }
      }
      if (maxHeight > 50) {
        int random = rand() % 21;
        if (random == 1)
          spawnTree(grid, glm::ivec3(col, maxHeight, row));
      }
    }
  }
  mSVDAG = RTVE::SparseVoxelDAG(grid);
  mSVDAG.translate(glm::vec3(mOffset.x, 0, mOffset.y));
  mSVDAG.mData.push_back({glm::vec4(0.1, 0.1, 0.1, 0)}); // Air - background color
  // mSVDAG.mData.push_back({glm::vec4(1, 1, 1, 0)}); // Block color
  mSVDAG.mData.push_back({glm::vec4(0.07, 0.63, 0.18, 0)}); // Grass
  mSVDAG.mData.push_back({glm::vec4(0.28, 0.59, 1.00, 0)}); // Water
  mSVDAG.mData.push_back({glm::vec4(0.37, 0.18, 0.14, 0)}); // Wood

  // mSVDAG.generateDebugMesh();
}

Chunk::~Chunk() {
  // mSVDAG.releaseDebugMesh();
}

void Chunk::spawnTree(std::vector<std::vector<std::vector<uint32_t>>>& pGrid, glm::ivec3 pPos) {
  pGrid.at(pPos.x).at(pPos.y).at(pPos.z) = 3;
  pGrid.at(pPos.x).at(pPos.y + 1).at(pPos.z) = 3;
  pGrid.at(pPos.x).at(pPos.y + 2).at(pPos.z) = 3;
  pGrid.at(pPos.x).at(pPos.y + 3).at(pPos.z) = 3;
  pGrid.at(pPos.x).at(pPos.y + 4).at(pPos.z) = 3;
}

