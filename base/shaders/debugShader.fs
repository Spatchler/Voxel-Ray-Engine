#version 460 core
out vec4 FragColor;

uniform vec3 uColour;

void main() {
  FragColor = vec4(uColour.r, uColour.g, uColour.b, 1);
}

