#pragma once

#include <glad/glad.h>
#include <stb/stb_image.h>
#include <array>
#include <string>
#include <print>

#include "utils.hpp"

namespace RTVE {
  class Skybox {
  public:
    Skybox(std::array<Path, 6> pFaces);

    void draw();

    void release();
  private:
    GLuint mVAO, mVBO, mTexID;
  };
}
