R""(
#version 400 core

layout (location = 0) in vec3 PositionVBO;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;

void main()
{
  gl_Position =
    ProjectionMatrix * ViewMatrix * ModelMatrix * vec4(PositionVBO, 1.f);
}
)"";
