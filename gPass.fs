R""(
#version 400 core
in vec3 vertexNormal;
in vec3 vertexPosition;
in vec3 vertexColor;
in vec2 textureCoords;

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gColor;

uniform sampler2D perlinNoise;

void main() {
  gPosition = vertexPosition;
 
  vec3 proceduralNormal = 
    texture2D(perlinNoise,
              textureCoords.xy
                * (gPosition.x * gPosition.y * gPosition.z)).rgb;
  
  proceduralNormal = proceduralNormal * 2 - vec3(1);
  //a more appealing way of doing this is needed.
  gNormal = normalize(vertexNormal + proceduralNormal / 2);
  
  gColor = vertexColor;
};
)"";
