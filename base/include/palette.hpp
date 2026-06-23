#pragma once

#include <vector>
#include <string>
#include <print>
#include <cstring>
#include <sstream>
#include <fstream>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <stb/stb_image.h>

namespace RTVE {
  struct ColourVoxelData {
    glm::vec4 color;
  };
  
  constexpr bool operator==(const ColourVoxelData& lhs, const ColourVoxelData& rhs) {
    return lhs.color == rhs.color;
  }

  #pragma pack(1)
  struct TextureVoxelData {
    uint32_t texTopIndex;
    uint32_t texBottomIndex;
    uint32_t texNSideIndex;
    uint32_t texSSideIndex;
    uint32_t texESideIndex;
    uint32_t texWSideIndex;
  };
  #pragma pack()
  
  class TexturePalette {
  public:
    TexturePalette(std::vector<std::string> pPaths, glm::ivec2 pTexSize, std::vector<TextureVoxelData> pData);

    constexpr uint getSize() {
      return mNumTextures;
    }

    GLuint getTexID();

    TextureVoxelData* getStart();
  private:
    GLuint mTexID;
    uint mNumTextures;
    std::vector<TextureVoxelData> mData;
  };

  class ColourPalette {
  public:
    ColourPalette(std::vector<ColourVoxelData> pData);
    ColourPalette(const std::string& pPath);

    constexpr uint getSize() {
      return mData.size();
    }

    ColourVoxelData* getStart();
  private:
    std::vector<ColourVoxelData> mData;
  };
}
