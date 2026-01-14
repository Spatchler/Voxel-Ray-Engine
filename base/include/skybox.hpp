#pragma once

#include <glad/glad.h>
#include <stb_image.h>
#include <array>
#include <string>
#include <print>

namespace RTVE {
  class Skybox {
  public:
    Skybox(std::array<std::string, 6> pFaces);

    void draw();

    void release();
  private:
    GLuint mVAO, mVBO, mTexID;
  };
}
