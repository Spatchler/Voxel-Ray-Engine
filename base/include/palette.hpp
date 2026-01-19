#pragma once

#include <vector>
#include <string>
#include <print>
#include <cstring>

#include <glm/glm.hpp>
#include <glad/glad.h>
#include <stb_image.h>

namespace RTVE {
  // struct VoxelData {
  //   glm::vec4 color;
  // };
  #pragma pack(1)
  struct VoxelData {
    uint32_t texTopIndex;
    uint32_t texSideIndex;
    uint32_t texBottomIndex;
    // uint32_t padding;
  };
  #pragma pack()
  
  // constexpr bool operator==(const VoxelData& lhs, const VoxelData& rhs) {
  //   return lhs.color == rhs.color;
  // }
  
  class Palette {
  public:
    Palette(std::vector<std::string> pPaths, glm::ivec2 pTexSize, std::vector<VoxelData> pData);

    // void add(VoxelData pItem);

    constexpr uint getSize() {
      return mNumTextures;
    }

    GLuint getTexID();

    VoxelData* getStart();
  private:
    GLuint mTexID;
    uint mNumTextures;
    std::vector<VoxelData> mData;
  };
}
