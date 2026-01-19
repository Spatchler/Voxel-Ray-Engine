#include "palette.hpp"

RTVE::Palette::Palette(std::vector<std::string> pPaths, glm::ivec2 pTexSize, std::vector<VoxelData> pData)
:mData(pData), mNumTextures(pPaths.size()) {
  std::vector<uint8_t> texData;
  // std::println();
  texData.resize(pTexSize.x * pTexSize.y * pPaths.size() * 4);

  for (uint i = 0; i < pPaths.size(); ++i) {
    int width, height, nrComponents;
    uint8_t* data = stbi_load(pPaths.at(i).c_str(), &width, &height, &nrComponents, 0);
    if (nrComponents != 4)
      std::println("Palette texture invalid number of components at path: {}", pPaths.at(i));
    if (data)
      std::memcpy(&texData.at(i*pTexSize.x*pTexSize.y*4), data, pTexSize.x*pTexSize.y*4);
    else
      std::println("Palette texture failed to load at path: {}", pPaths.at(i));
    stbi_image_free(data);
  }

  glGenTextures(1, &mTexID);
  glBindTexture(GL_TEXTURE_2D_ARRAY, mTexID);

  glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, pTexSize.x, pTexSize.y, pPaths.size());

  glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, pTexSize.x, pTexSize.y, pPaths.size(), GL_RGBA, GL_UNSIGNED_BYTE, &texData.at(0));
  
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

GLuint RTVE::Palette::getTexID() {
  return mTexID;
}

// void RTVE::Palette::add(RTVE::VoxelData pItem) {
//   mData.emplace_back(pItem);
// }

RTVE::VoxelData* RTVE::Palette::getStart() {
  return &mData.at(0);
}

