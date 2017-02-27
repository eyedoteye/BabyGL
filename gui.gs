R""(
#version 400 core
out vec3 boxColor;
in VS_OUT
{
  float t;
  float barWidth;
  vec2 dimensions;
  int isSelected;
} gs_in[];

layout (points) in;
layout (triangle_strip, max_vertices = 12) out;

uniform float onePixel;

void buildSquare(vec4 position, float width, float height)
{
 gl_Position = position;
 EmitVertex();
 gl_Position = position + vec4(width, 0.f, 0.f, 0.f);
 EmitVertex();
 gl_Position = position + vec4(0.f, -height, 0.f, 0.f);
 EmitVertex();
 gl_Position = position + vec4(width, -height, 0.f, 0.f);
 EmitVertex();
 EndPrimitive();
}

void main()
{
  float width = gs_in[0].dimensions.x;
  float barWidth = gs_in[0].barWidth;
  float t = gs_in[0].t * (width - barWidth - onePixel * 2);
  float height = gs_in[0].dimensions.y;
  
  boxColor = vec3(1.f); 
  buildSquare(gl_in[0].gl_Position, t, height);
  buildSquare(gl_in[0].gl_Position
                + vec4(t + onePixel * 2 + barWidth, vec3(0.f)),
              width - t - onePixel * 2 - barWidth, height);
  
  if(gs_in[0].isSelected != 0)
  {
    boxColor = vec3(0.f);
  }
  
  buildSquare(gl_in[0].gl_Position
    + vec4(t + onePixel, vec3(0.f)),
  barWidth, height);
}
)"";
