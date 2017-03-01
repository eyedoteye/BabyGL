R""(
#version 400 core

out vec2 textureCoords;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tCoords; 

void main()
{
  gl_Position = vec4(position, 1.f);
  textureCoords = tCoords;
}
)"";
