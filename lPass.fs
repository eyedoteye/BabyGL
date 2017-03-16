R""(
#version 400 core

#define MAX_DIRECTIONAL_LIGHTS 1
struct DirectionalLight {
  vec3 color;
  vec3 direction;

  float intensityAmbient;
  float intensityDiffuse;
  float intensitySpecular;
};
uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS]; 

struct PointLight
{
  vec3 color;
  vec3 position;

  float attenuationLinear;
  float attenuationQuadratic;

  float intensityAmbient;
  float intensityDiffuse;
  float intensitySpecular;
}; 
#define MAX_POINT_LIGHTS 2
layout (std140) uniform pointLightsUBO
{
  PointLight pointLights[MAX_POINT_LIGHTS];  
};

out vec4 fragmentColor;
in vec2 textureCoords;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D colorBuffer;
uniform vec3 viewPosition;

vec3 computeDirectionalLightContribution(
  DirectionalLight directionalLight,
  vec3 objectColor,
  vec3 normal,
  vec3 fragmentPosition,
  vec3 viewDirection
) {
  vec3 lightDirection = normalize(-directionalLight.direction);
  vec3 reflectDirection = reflect(-lightDirection, normal);
  
  vec3 ambient = directionalLight.intensityAmbient * objectColor;
  vec3 diffuse = directionalLight.intensityDiffuse * directionalLight.color
               * max(dot(normal, lightDirection), 0.f) * objectColor;
  vec3 specular = directionalLight.intensitySpecular * directionalLight.color
                * pow(max(dot(viewDirection, reflectDirection), 0.f), 64);
                //Currently, no specular map.

  return ambient + diffuse + specular;
}

vec3 computePointLightContribution(
  PointLight pointLight,
  vec3 objectColor,
  vec3 normal,
  vec3 fragmentPosition,
  vec3 viewDirection
) {
  vec3 lightDirection = normalize(pointLight.position - fragmentPosition);
  vec3 reflectDirection = reflect(-lightDirection, normal);

  vec3 ambient = pointLight.intensityAmbient * objectColor;
  vec3 diffuse = pointLight.intensityDiffuse * pointLight.color
               * max(dot(normal, lightDirection), 0.f) * objectColor;
  vec3 specular = pointLight.intensitySpecular * pointLight.color
                * pow(max(dot(viewDirection, reflectDirection), 0.f), 64);
                //Currently, no specular map.

  float distance = length(pointLight.position - fragmentPosition);
  float attenuation = 1.f / (1.f
                               + pointLight.attenuationLinear * distance
                               + pointLight.attenuationQuadratic * distance * distance);

  return (ambient + diffuse + specular) * attenuation;
}

//This rand function is copy/pasted from interwebs.
//http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
highp float rand(vec2 co) {
  highp float a = 12.9898;
  highp float b = 78.233;
  highp float c = 43758.5433;
  highp float dt = dot(co.xy, vec2(a,b));
  highp float sn = mod(dt, 3.14);

  return fract(sin(sn) * c);
}

vec4 computeColor(
  vec3 normal,
  vec3 objectColor,
  vec3 fragmentPosition,
  vec3 viewDirection)
{
    vec3 result = .1f * objectColor;
    for(int DirectionalLightIndex = 0;
        DirectionalLightIndex < MAX_DIRECTIONAL_LIGHTS;
        ++DirectionalLightIndex)
    {
      result += computeDirectionalLightContribution(
        directionalLights[DirectionalLightIndex],
        objectColor,
        normal,
        fragmentPosition,
        viewDirection);
    }
    for(int PointLightIndex = 0;
        PointLightIndex < MAX_POINT_LIGHTS;
        ++PointLightIndex)
    {
      result += computePointLightContribution(
        pointLights[PointLightIndex],
        objectColor,
        normal,
        fragmentPosition,
        viewDirection);
    }

    return vec4(result + mix(-0.5f/255, 0.5f/255,
                rand(textureCoords)), 1.f);
}

subroutine vec4 debugRoutine();
subroutine uniform debugRoutine currentRoutine;
uniform sampler2D lightColorBuffer;

subroutine(debugRoutine) vec4 debugOn()
{
  vec3 normal = texture(normalBuffer, textureCoords).rgb;
  vec4 lightColor = texture(lightColorBuffer, textureCoords);          
  if(normal == vec3(0.f) && lightColor.a == 0.f)
    discard;

  vec3 objectColor = texture(colorBuffer, textureCoords).rgb;
  vec3 fragmentPosition = texture(positionBuffer, textureCoords).rgb;
  vec3 viewDirection = normalize(viewPosition - fragmentPosition);
    
  vec4 sceneColor = computeColor(normal, objectColor, fragmentPosition, viewDirection);

  vec4 finalColor = vec4(1.f);

  //Note: This is a special case that exists because I'm using a
  //2D background. In the future, actual blending may be needed if a skybox is
  //used.
  if(normal == vec3(0.f) && lightColor.a < 1.f)
    finalColor.rgb = vec3(.4f, .6f, .2f) * (1 - lightColor.a)
                   + lightColor.rgb;
  else
    finalColor.rgb = sceneColor.rgb * (1 - lightColor.a)
                   + lightColor.rgb; 
  return finalColor; 
}

subroutine(debugRoutine) vec4 debugOff()
{                    
  vec3 normal = texture(normalBuffer, textureCoords).rgb;
  if(normal == vec3(0.f))
    discard;

  vec3 objectColor = texture(colorBuffer, textureCoords).rgb;
  vec3 fragmentPosition = texture(positionBuffer, textureCoords).rgb;
  vec3 viewDirection = normalize(viewPosition - fragmentPosition);
    
  return computeColor(normal, objectColor, fragmentPosition, viewDirection);
}


void main() {
  fragmentColor = currentRoutine();
}
)"";
