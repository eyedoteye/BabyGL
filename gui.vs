R""(
#version 400 core
out VS_OUT
{
 float t;
 float barWidth;
 vec2 dimensions;
 int isSelected;
}vs_out;

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 dimensions;
layout (location = 2) in float barWidth;
layout (location = 3) in float t;
layout (location = 4) in int isSelected;

void main()
{
  gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);
  vs_out.t = t;
  vs_out.dimensions = dimensions;
  vs_out.barWidth = barWidth;
  vs_out.isSelected = isSelected; 
};
)"";
