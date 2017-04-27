R""(
#version 400 core
out vec4 FragmentColor;

in vec3 BarColor;

void
main()
{
  FragmentColor = vec4(BarColor, 1.f);
}
)"";
