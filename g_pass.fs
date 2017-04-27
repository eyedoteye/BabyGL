R""(
#version 400 core

in vec3 VertexNormal;
in vec3 VertexPosition;
in vec3 VertexColor;
in vec2 UV;

layout (location = 0) out vec3 PositionTexture;
layout (location = 1) out vec3 NormalTexture;
layout (location = 2) out vec3 ColorTexture;
layout (location = 3) out vec4 PointLightColorTexture; 

uniform sampler2D PerlinNoise;
uniform int ObjectType;

#define OBJECT_TYPE_NONE 0
#define OBJECT_TYPE_DEFAULT 1
#define OBJECT_TYPE_POINTLIGHT 2

void
main()
{
  PointLightColorTexture = vec4(0.f);
  ColorTexture = vec3(0.f);
  NormalTexture = vec3(0.f);
  switch(ObjectType)
  {
    case OBJECT_TYPE_POINTLIGHT:
    {
      PointLightColorTexture = vec4(VertexColor, 1.f);
    } break;
    case OBJECT_TYPE_DEFAULT:
    {
      vec3 ProceduralNormal =
        texture(PerlinNoise,
                UV.xy
                * (VertexPosition.x * VertexPosition.y * VertexPosition.z)).rgb;
      
      ProceduralNormal = ProceduralNormal * 2 - vec3(1);
      // Note: A more aesthetic way of doing this is needed.
      NormalTexture = normalize(VertexNormal + ProceduralNormal / 2);

      PositionTexture = VertexPosition;
      ColorTexture = VertexColor;
    } break;
  }
}
)"";
