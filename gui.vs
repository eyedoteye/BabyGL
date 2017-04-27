R""(
#version 400 core
out vs_out
{
 float Value;
 float ThumbWidth;
 vec2 Dimensions;
 int IsSelected;
} VSOut;

layout (location = 0) in vec2 Position;
layout (location = 1) in vec2 Dimensions;
layout (location = 2) in float ThumbWidth;
layout (location = 3) in float Value;
layout (location = 4) in int IsSelected;

void
main()
{
  gl_Position = vec4(Position.x, Position.y, 0.0f, 1.0f);
  VSOut.Value = Value;
  VSOut.Dimensions = Dimensions;
  VSOut.ThumbWidth = ThumbWidth;
  VSOut.IsSelected = IsSelected; 
};
)"";
