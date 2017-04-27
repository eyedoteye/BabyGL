R""(
#version 400 core

in vec2 UV;

layout (location = 0) out vec4 ColorVBO;

uniform sampler2D ColorTexture;

subroutine vec4 PointLightRoutine(vec4 color);
subroutine uniform PointLightRoutine CurrentRoutine;

// Note: Weights taken from https://learnopengl.com/#!Advanced-Lighting/Bloom
uniform float GaussianWeights[5] =
  float[] (0.277027, 0.1945946, 0.1216216, 0.054054, 0.016216);

// Note: Set one component of TexelSize to 0 for blurring in the other axis.
vec4
GaussianBlur(vec4 OriginColor, vec2 TexelSize)
{
  vec4 BlurredColor = OriginColor * GaussianWeights[0];
  for(int WeightIndex = 1; WeightIndex < 5; ++WeightIndex)
  {
    BlurredColor += texture(ColorTexture,
                      UV + vec2(TexelSize.x * WeightIndex,
                                TexelSize.y * WeightIndex))
                      * GaussianWeights[WeightIndex];
    BlurredColor += texture(ColorTexture,
                      UV - vec2(TexelSize.x * WeightIndex,
                                TexelSize.y * WeightIndex))
                      * GaussianWeights[WeightIndex];
  }
  return BlurredColor;
}

subroutine (PointLightRoutine) vec4
BlurHorizontal(vec4 OriginColor)
{
  vec2 TexelSize = 1.f / textureSize(ColorTexture, 0);
  TexelSize.y = 0.f;
  return GaussianBlur(OriginColor, TexelSize);
}

subroutine (PointLightRoutine) vec4
BlurVertical(vec4 OriginColor)
{
  vec2 TexelSize = 1.f / textureSize(ColorTexture, 0);
  TexelSize.x = 0.f;
  return GaussianBlur(OriginColor, TexelSize);
}

void
main()
{
  ColorVBO = CurrentRoutine(texture(ColorTexture, UV));
}
)"";
