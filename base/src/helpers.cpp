#include "helpers.hpp"

float RTVE::v3index(glm::tvec3<float> v, uint8_t i) {
  if (i == 0)
    return v.x;
  if (i == 1)
    return v.y;
  if (i == 2)
    return v.z;
  return 0.f;
}

void RTVE::printProgressBar(float pProgress, const std::string& pTitle, uint pWidth) {
  std::print("\x1b[1F\x1b[2K{} |", pTitle);
  float threshold = ceil(pProgress * pWidth);
  std::print("\x1b[41m");
  for (uint i = 0; i < threshold; ++i) {
    std::print(" ");
  }
  std::print("\x1b[40m");
  for (uint i = 0; i < pWidth - threshold; ++i) {
    std::print(" ");
  }
  std::println("\x1b[49m| {}%", ceil(pProgress*100)); // Reset
  // std::println("");
}

