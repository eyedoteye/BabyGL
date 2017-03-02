R""(
#version 400 core
in vec3 vertexNormal;
in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 textureCoords;

layout (location = 0) out vec3 positionBuffer;
layout (location = 1) out vec3 normalBuffer;
layout (location = 2) out vec4 colorBuffer;
layout (location = 3) out vec3 pointLightColorBuffer; 
layout (location = 4) out int typeBuffer;

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
      colorBuffer.a = 0.f;
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
      colorBuffer = vec4(vertexColor, 1.f);
      
      // Note: sets bg color, better method is needed.
      pointLightColorBuffer = vec3(.4f, .6f, .2f);
    } break;
  }
}
)"";
