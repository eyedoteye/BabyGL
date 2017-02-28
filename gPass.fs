R""(
#version 400 core
in vec3 vertexNormal;
in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 textureCoords;

layout (location = 0) out vec3 positionBuffer;
layout (location = 1) out vec3 normalBuffer;
layout (location = 2) out vec3 colorBuffer;
layout (location = 3) out vec3 pointLightPositionBuffer;
layout (location = 4) out vec3 pointLightColorBuffer; 

uniform sampler2D perlinNoise;
uniform int objectType;

#define OBJECT_TYPE_DEFAULT 0
#define OBJECT_TYPE_POINTLIGHT 1

void main()
{
  switch(objectType)
  {
    case OBJECT_TYPE_POINTLIGHT:
    {
      normalBuffer = vec3(0.f);
      pointLightPositionBuffer = vertexPosition;
      pointLightColorBuffer = vertexColor;
    } break;
    default:
    {
      vec3 proceduralNormal = texture(perlinNoise,
                                        textureCoords.xy
                                          * (vertexPosition.x
                                          * vertexPosition.y * vertexPosition.z)).rgb;
      
      proceduralNormal = proceduralNormal * 2 - vec3(1);
      //a more appealing way of doing this is needed.
      normalBuffer = normalize(vertexNormal + proceduralNormal / 2);

      positionBuffer = vertexPosition;
      colorBuffer = vertexColor;
    } break;
  }
}
)"";
