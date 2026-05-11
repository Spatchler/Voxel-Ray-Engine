#include "palette.hpp"

RTVE::TexturePalette::TexturePalette(std::vector<std::string> pPaths, glm::ivec2 pTexSize, std::vector<TextureVoxelData> pData)
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

GLuint RTVE::TexturePalette::getTexID() {
  return mTexID;
}

RTVE::TextureVoxelData* RTVE::TexturePalette::getStart() {
  return &mData.at(0);
}

RTVE::ColourPalette::ColourPalette(std::vector<RTVE::ColourVoxelData> pData)
:mData(pData) {
}

RTVE::ColourPalette::ColourPalette(const std::string& pPath) {
  // Open
  std::ifstream fin;
  fin.open(pPath, std::ios::binary | std::ios::in);
  if (!fin.is_open()) throw std::invalid_argument("Could not open input palette file");
  // Read
  uint numColours;
  std::string line;
  std::getline(fin, line);
  if (line != "JASC-PAL") throw std::invalid_argument("Palette file incorrect format");
  std::getline(fin, line);
  if (line != "0100") throw std::invalid_argument("Palette file incorrect format version");
  std::getline(fin, line);
  numColours = std::stoi(line);
  mData.resize(numColours);
  uint count = 0;
  for (; std::getline(fin, line) && (count < numColours); ++count) {
    std::stringstream s;
    s << line;
    RTVE::ColourVoxelData& c = mData.at(count);
    std::string componentStr;
    for (uint i = 0; i < 3; ++i) {
      std::getline(s, componentStr, ' ');
      c.color[i] = std::stoi(componentStr) / 255.f;
    }
  }
  if (count != numColours) throw std::overflow_error("Palette file invalid number of colours");
  // Close
  fin.close();
}

RTVE::ColourVoxelData* RTVE::ColourPalette::getStart() {
  return &mData.at(0);
}

