R""(
#version 400 core
out vec4 fragmentColor;
in vec3 boxColor;

void main()
{
  fragmentColor = vec4(boxColor, 1.f);
}
)"";
