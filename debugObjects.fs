R""(
#version 400 core

out vec4 fragmentColor;
in vec2 textureCoords;

uniform sampler2D colorBuffer;

void main()
{
  fragmentColor = vec4(texture(colorBuffer, textureCoords).rgb, 1.f);
}
)"";
