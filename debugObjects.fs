R""(
#version 400 core

out vec4 fragmentColor;
in vec2 textureCoords;

uniform sampler2D colorBuffer;

subroutine vec4 pointLightRoutine(vec4 color);
subroutine uniform pointLightRoutine currentRoutine;

// Weights taken from https://learnopengl.com/#!Advanced-Lighting/Bloom
uniform float weight[5] = float[] (0.277027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// Set one component of texelSize to 0 for blurring in the other axis.
vec4 gaussianBlur(vec4 originColor, vec2 texelSize)
{
  vec4 result = originColor * weight[0];
  for(int weightIndex = 1; weightIndex < 5; ++weightIndex)
  {
    result += texture(colorBuffer,
                      textureCoords + vec2(texelSize.x * weightIndex,
                                           texelSize.y * weightIndex)).rgb
                      * weight[weightIndex];
    result += texture(colorBuffer,
                      textureCoords - vec2(texelSize.x * weightIndex,
                                           texelSize.y * weightIndex)).rgb
                      * weight[weightIndex];
  }
  return result;
}

subroutine (pointLightRoutine) vec4 blurHorizontal(vec4 originColor)
{
  vec2 texelSize = 1.f / textureSize(colorBuffer, 0);
  texelSize.y = 0.f;
  return gaussianBlur(originColor, texelSize);
}
subroutine (pointLightRoutine) vec4 blurVertical(vec4 originColor)
{
  vec2 texelSize = 1.f / textureSize(colorBuffer, 0);
  texelSize.x = 0.f;
  return gaussianBlur(originColor, texelSize);
}

subroutine (pointLightRoutine) vec4 discardNonSpheres(vec4 color)
{
  if(color.a == 0.f)
    discard;

  return color;
}

void main()
{
  vec4 color = texture(colorBuffer, textureCoords); 
  fragmentColor = currentRoutine(color);
}
)"";
