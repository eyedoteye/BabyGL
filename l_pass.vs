R""(
#version 400 core

out vec2 UV;

layout (location = 0) in vec3 PositionVBO;
layout (location = 1) in vec2 UVVBO;

void
main() {
  gl_Position = vec4(PositionVBO, 1.f);
  UV = UVVBO;
}
)"";
