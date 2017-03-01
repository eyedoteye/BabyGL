R""(
#version 400 core

out vec4 fragmentColor;
in vec2 textureCoords;

uniform sampler2D colorBuffer;

void main()
{
  fragmentColor = texture(colorBuffer, textureCoords);
}
)"";
