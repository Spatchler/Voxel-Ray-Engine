#pragma once

#include <glm/glm.hpp>
#include <print>

namespace RTVE {
  float v3index(glm::tvec3<float> v, uint8_t i);

  void printProgressBar(float pProgress, const std::string& pTitle, uint pWidth = 20);
}

