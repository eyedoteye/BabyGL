R""(
#version 400 core

#define MAX_DIRECTIONAL_LIGHTS 1
struct directional_light {
  vec3 Color;
  vec3 Direction;

  float IntensityAmbient;
  float IntensityDiffuse;
  float IntensitySpecular;
};
uniform directional_light DirectionalLights[MAX_DIRECTIONAL_LIGHTS]; 

struct point_light
{
  //Note: The following are actually vec3s,
  //but are declared as vec4 to reduce (std140) UBO bugs.
  vec4 Color;      
  vec4 Position;

  float AttenuationLinear;
  float AttenuationQuadratic;

  float IntensityAmbient;
  float IntensityDiffuse;
  float IntensitySpecular;
}; 
#define MAX_POINT_LIGHTS 2
layout (std140) uniform PointLightUBO
{
  point_light PointLights[MAX_POINT_LIGHTS];  
};

out vec4 FragmentColor;
in vec2 UV;

uniform sampler2D PositionTexture;
uniform sampler2D NormalTexture;
uniform sampler2D ColorTexture;
uniform vec3 ViewPosition;

vec3
ComputeDirectionalLightContribution(
  directional_light DirectionalLight,
  vec3 ObjectColor,
  vec3 Normal,
  vec3 FragmentPosition,
  vec3 ViewDirection
) {
  vec3 LightDirection = normalize(-DirectionalLight.Direction);
  vec3 ReflectDirection = reflect(-LightDirection, Normal);
  
  vec3 Ambient = DirectionalLight.IntensityAmbient * ObjectColor;
  vec3 Diffuse = DirectionalLight.IntensityDiffuse * DirectionalLight.Color
                 * max(dot(Normal, LightDirection), 0.f) * ObjectColor;
  vec3 Specular = DirectionalLight.IntensitySpecular * DirectionalLight.Color
                  * pow(max(dot(ViewDirection, ReflectDirection), 0.f), 64);
 // Note: No Specular map

  return Ambient + Diffuse + Specular;
}

vec3
ComputePointLightContribution(
  point_light PointLight,
  vec3 ObjectColor,
  vec3 Normal,
  vec3 FragmentPosition,
  vec3 ViewDirection
) {
  vec3 LightDirection = normalize(PointLight.Position.xyz - FragmentPosition);
  vec3 ReflectDirection = reflect(-LightDirection, Normal);

  vec3 Ambient = PointLight.IntensityAmbient * ObjectColor;
  vec3 Diffuse = PointLight.IntensityDiffuse * PointLight.Color.rgb
               * max(dot(Normal, LightDirection), 0.f) * ObjectColor;
  vec3 Specular = PointLight.IntensitySpecular * PointLight.Color.rgb
                * pow(max(dot(ViewDirection, ReflectDirection), 0.f), 64);

  // Note: No Specular map

  float distance = length(PointLight.Position.xyz - FragmentPosition);
  float Attenuation =
    1.f / (1.f + PointLight.AttenuationLinear * distance
           + PointLight.AttenuationQuadratic * distance * distance);

  return (Ambient + Diffuse + Specular) * Attenuation;
}

// Note: This rand function is from interwebs
// byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float
rand(vec2 Seed) {
  highp float A = 12.9898;
  highp float B = 78.233;
  highp float C = 43758.5433;
  highp float DT = dot(Seed.xy, vec2(A,B));
  highp float SN = mod(DT, 3.14);

  return fract(sin(SN) * C);
}

vec4
ComputeLighting(
  vec3 Normal,
  vec3 ObjectColor,
  vec3 FragmentPosition,
  vec3 ViewDirection)
{
    vec3 LightedColor = .1f * ObjectColor;
    for(int DirectionalLightIndex = 0;
        DirectionalLightIndex < MAX_DIRECTIONAL_LIGHTS;
        ++DirectionalLightIndex)
    {
      LightedColor += ComputeDirectionalLightContribution(
        DirectionalLights[DirectionalLightIndex],
        ObjectColor,
        Normal,
        FragmentPosition,
        ViewDirection);
    }
    for(int PointLightIndex = 0;
        PointLightIndex < MAX_POINT_LIGHTS;
        ++PointLightIndex)
    {
      LightedColor += ComputePointLightContribution(
        PointLights[PointLightIndex],
        ObjectColor,
        Normal,
        FragmentPosition,
        ViewDirection);
    }

    return vec4(LightedColor + mix(-0.5f/255, 0.5f/255,
                rand(UV)), 1.f);
}

subroutine vec4 LightingRoutine();
subroutine uniform LightingRoutine CurrentLightingRoutine;
uniform sampler2D PointLightColorTexture;

subroutine(LightingRoutine) vec4
DebugOn()
{
  vec3 Normal = texture(NormalTexture, UV).rgb;
  vec4 LightColor = texture(PointLightColorTexture, UV);          

  if(Normal == vec3(0.f) && LightColor.a == 0.f)
    discard;

  vec3 ObjectColor = texture(ColorTexture, UV).rgb;
  vec3 FragmentPosition = texture(PositionTexture, UV).rgb;
  vec3 ViewDirection = normalize(ViewPosition - FragmentPosition);
    
  vec4 SceneColor =
    ComputeLighting(Normal, ObjectColor, FragmentPosition, ViewDirection);

  vec4 FinalColor = vec4(1.f);

  // Note: Special case because the Clear Color is used as the background
  // Todo: Have the Clear Color passed in
  if(Normal == vec3(0.f) && LightColor.a < 1.f)
    FinalColor.rgb = vec3(.4f, .6f, .2f) * (1 - LightColor.a) + LightColor.rgb;
  else
    FinalColor.rgb = SceneColor.rgb * (1 - LightColor.a) + LightColor.rgb; 
  return FinalColor; 
}

subroutine(LightingRoutine) vec4
DebugOff()
{                    
  vec3 Normal = texture(NormalTexture, UV).rgb;
  if(Normal == vec3(0.f))
    discard;

  vec3 ObjectColor = texture(ColorTexture, UV).rgb;
  vec3 FragmentPosition = texture(PositionTexture, UV).rgb;
  vec3 ViewDirection = normalize(ViewPosition - FragmentPosition);
    
  return ComputeLighting(Normal, ObjectColor, FragmentPosition, ViewDirection);
}

void
main() {
  FragmentColor = CurrentLightingRoutine();
}
)"";
