R""(
#version 400 core
out vec3 BarColor;

in vs_out
{
  float Value;
  float ThumbWidth;
  vec2 Dimensions;
  int IsSelected; // No bool in GLSL
} GSIn[];

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

uniform float OnePixel;

void
BuildQuad(vec4 Position, float Width, float Height)
{
  gl_Position = Position;
  EmitVertex();
  gl_Position = Position + vec4(Width, 0.f, 0.f, 0.f);
  EmitVertex();
  gl_Position = Position + vec4(0.f, -Height, 0.f, 0.f);
  EmitVertex();
  gl_Position = Position + vec4(Width, -Height, 0.f, 0.f);
  EmitVertex();
  EndPrimitive();
}

void
main()
{
  float Width = GSIn[0].Dimensions.x;
  float ThumbWidth = GSIn[0].ThumbWidth;
  float Value = GSIn[0].Value * (Width - ThumbWidth - OnePixel * 2);
  float Height = GSIn[0].Dimensions.y;
  
  BarColor = vec3(1.f); 
  BuildQuad(gl_in[0].gl_Position, Value, Height);
  BuildQuad(gl_in[0].gl_Position
                + vec4(Value + OnePixel * 2 + ThumbWidth, vec3(0.f)),
              Width - Value - OnePixel * 2 - ThumbWidth, Height);
  
  if(GSIn[0].IsSelected != 0)
  {
    BarColor = vec3(0.f);
  }
  
  BuildQuad(gl_in[0].gl_Position
    + vec4(Value + OnePixel, vec3(0.f)),
  ThumbWidth, Height);
}
)"";
