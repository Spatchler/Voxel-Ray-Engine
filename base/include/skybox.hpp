#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <array>
#include <string>
#include <print>
#include <filesystem>

namespace RTVE {
  class Skybox {
  public:
    Skybox(std::array<std::filesystem::path, 6> pFaces);

    void draw();

    void release();
  private:
    GLuint mVAO, mVBO, mTexID;
  };
}
