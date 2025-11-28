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

void RTVE::printProgressBar(float pProgress, const std::string& pTitle, uint8_t pPrimaryEscapeColour, uint8_t pSecondaryEscapeColour, uint pWidth) {
  std::print("\x1b[1F\x1b[2K{} ", pTitle);
  float threshold = ceil(pProgress * pWidth);
  std::print("\x1b[{}m\x1b[{}m", pPrimaryEscapeColour, pPrimaryEscapeColour + 10);
  for (uint i = 0; i < threshold; ++i) {
    std::print(" ");
  }
  if (threshold <= pWidth - 1) {
    std::print("\x1b[{}m\x1b[{}m", pSecondaryEscapeColour + 10, pSecondaryEscapeColour);
    for (uint i = 0; i < pWidth - threshold - 1; ++i) {
      std::print(" ");
    }
    std::print("\x1b[49m");
  }
  else {
    std::print("\x1b[49;{}m", pPrimaryEscapeColour);
  }
  std::println("\x1b[39m {}%", ceil(pProgress*100)); // Reset
}

