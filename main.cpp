#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma warning(push)
#pragma warning(disable:4244 4996 4305 4838 4456 4100)
#include <par/par_shapes.h>
#pragma warning(pop)

#include "RenderObject.cpp"
#include "ShaderObject.cpp"
#include "Camera.cpp"

#include <stdlib.h>
#include <time.h>

struct DirectionalLight
{
  glm::vec3 direction;
  glm::vec3 color;
  struct
  {
    float ambient;
    float diffuse;
    float specular;
  } intensity;
};

struct PointLight
{
  glm::vec3 position;
  glm::vec3 color;
  struct
  {
    float ambient;
    float diffuse;
    float specular;
  } intensity;
  struct
  {
    float linear;
    float quadratic;
  } attenuation;
};

void drawPointLightDebugModel(RenderObject debugModel, PointLight* pointLight, GLuint shaderProgramID)
{
  debugModel.model = glm::translate(debugModel.model, pointLight->position);
  debugModel.color = pointLight->color;
  drawRenderObject(&debugModel, shaderProgramID);
}

struct SliderGUI
{
  GLuint VAO;
  GLuint vboPositions, vboTs;

  int count;
  float* positions;
  float* ts;
};

void initSliderGUI(SliderGUI *sliderGUI, float* positions, float* ts, int count)
{
  glGenVertexArrays(1, &sliderGUI->VAO);
  glGenBuffers(1, &sliderGUI->vboPositions);
  glGenBuffers(1, &sliderGUI->vboTs);

  sliderGUI->positions = positions;
  sliderGUI->ts = ts;

  glBindVertexArray(sliderGUI->VAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, sliderGUI->vboPositions);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * count * 5,
                 positions,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5,
                          (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5,
                          (GLvoid*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE,
                          sizeof(float) * 5,
                          (GLvoid*)(4 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER,  sliderGUI->vboTs);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * count,
                 ts,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
                          sizeof(float),
                          (GLvoid*)0);
    glEnableVertexAttribArray(3);
  }
}

float lerp(float v0, float v1, float t)
{
  return (1.f - t) * v0 + t * v1;
}
#define TEXTURE_SIZE 128

static char charPermutation[255];
void initCharPermutation(int seed)
{
  srand(seed);
  for(int index = 0; index < 255; ++index)
  {
    charPermutation[index] = (char)index;
  }

#define ITERATIONS 3000
  for(int iteration = 0; iteration < ITERATIONS; ++iteration)
  {
    char temp;
    int randomIndex1 = rand() % 255;
    int randomIndex2 = rand() % 255;

    temp = charPermutation[randomIndex1];
    charPermutation[randomIndex1] = charPermutation[randomIndex2];
    charPermutation[randomIndex2] = temp;
  }
}

float computePerlinInfluence(int hash, float x, float y)
{
  switch(hash & 0x3)
  {
    case 0x0: return x + y;
    case 0x1: return -x + y;
    case 0x2: return x - y;
    case 0x3: return -x - y;
#define THEOMEGAVARIABLE 3.14f
    default: return THEOMEGAVARIABLE;
  }
}

float fade(float t)
{
  return t * t * t * (t * (t * 6 - 15) + 10);
}

float computePerlinNoise(float x, float y)
{
  while (x > 255)
  {
    x = x - 255;
  }
  while (y > 255)
  {
    y = y - 255;
  }

  int xi = (int)x & 255;
  int yi = (int)y & 255;

  float xf = x - (int)x;
  float yf = y - (int)y;

  float u = fade(xf);
  float v = fade(yf);

  int aa = charPermutation[(charPermutation[xi] + yi) % 255];
  int ab = charPermutation[(charPermutation[xi] + yi + 1) % 255];
  int ba = charPermutation[(charPermutation[(xi + 1) % 255] + yi) % 255];
  int bb = charPermutation[(charPermutation[(xi + 1) % 255] + yi + 1) % 255];

  float v1 = lerp(computePerlinInfluence(aa, xf, yf),
                  computePerlinInfluence(ba, xf - 1, yf),
                  u);
  float v2 = lerp(computePerlinInfluence(ab, xf, yf - 1),
                  computePerlinInfluence(bb, xf - 1, yf -1),
                  u);

  float vv = (lerp(v1, v2, v) + 1)/2;

  return vv;
}

static float textureData[TEXTURE_SIZE][TEXTURE_SIZE][3];

void generate2DPerlinNoise(GLuint textureID, int seed)
{
  initCharPermutation(seed);

  for(int row = 0; row < TEXTURE_SIZE; ++row)
  {
    for(int col = 0; col < TEXTURE_SIZE; ++col)
    {
      textureData[row][col][0] =
      textureData[row][col][1] =
        textureData[row][col][2] = computePerlinNoise((float)col/2, (float)row/2);
    }
  }

  glBindTexture(GL_TEXTURE_2D, textureID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, TEXTURE_SIZE, TEXTURE_SIZE, 0,
               GL_RGB, GL_FLOAT, (GLvoid*)textureData);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, 0);
}

#pragma warning(push)
#pragma warning(disable:4100)
static bool keys[1024];
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if(key == GLFW_KEY_ESCAPE)
  {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if(action == GLFW_PRESS)
  {
    keys[key] = true;
  }
  else if(action == GLFW_RELEASE)
  {
    keys[key] = false;
  }
}

static GLfloat mouseCoords[2];
void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
  mouseCoords[0] = (GLfloat)xpos;
  mouseCoords[1] = (GLfloat)ypos;
}
#pragma warning(pop)


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Duckbut", NULL, NULL);
  if(!window)
  {
    printf("GLFW window creation failure.\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(window);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    printf("GLEW initialization failure.\n");
    glfwTerminate();
    return -1;
  }

  glfwSetKeyCallback(window, keyCallback);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  int width, height;
  glfwGetFramebufferSize(window, &width, &height);
  glViewport(0, 0, width, height);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glEnable(GL_DEPTH_TEST);

  GLchar* gPassVertexShaderSource =
    "#version 400 core\n"
    "out vec3 vertexNormal;"
    "out vec3 vertexPosition;"
    "out vec3 vertexColor;"
    "out vec2 textureCoords;"

    "layout (location = 0) in vec3 position;"
    "layout (location = 1) in vec3 normal;"
    "layout (location = 2) in vec2 uvCoords;"

    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    "uniform vec3 objectColor;"

    "void main()"
    "{"
    " gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.f);"
    " vertexNormal = mat3(transpose(inverse(model))) * normal;"
    " vertexPosition = vec3(model * vec4(position, 1.f));"
    " vertexColor = objectColor;"
    " textureCoords = uvCoords;"
    "}";

  GLchar* gPassFragmentShaderSource =
    "#version 400 core\n"
    "in vec3 vertexNormal;"
    "in vec3 vertexPosition;"
    "in vec3 vertexColor;"
    "in vec2 textureCoords;"

    "layout (location = 0) out vec3 gPosition;"
    "layout (location = 1) out vec3 gNormal;"
    "layout (location = 2) out vec3 gColor;"

    "uniform sampler2D perlinNoise;"

    "void main()"
    "{"
    " gPosition = vertexPosition;"
    " vec3 proceduralNormal = texture2D(perlinNoise,"
    "                                   textureCoords.xy*(gPosition.x * gPosition.y * gPosition.z)).rgb;"
    " proceduralNormal = proceduralNormal * 2 - vec3(1);"
    //a more appealing way of doing this is needed.
    " gNormal = normalize(vertexNormal + proceduralNormal / 2);"
    //" gNormal = vertexPosition;"
    " gColor = vertexColor;"
    "}";

  GLchar* lPassVertexShaderSource =
    "#version 400 core\n"
    "out vec2 textureCoords;"

    "layout (location = 0) in vec3 position;"
    "layout (location = 1) in vec2 tCoords;"

    "void main()"
    "{"
    " gl_Position = vec4(position, 1.f);"
    " textureCoords = tCoords;"
    "}";

  GLchar* lPassFragmentShaderSource =
    "#version 400 core\n"

    "#define MAX_DIRECTIONAL_LIGHTS 1\n"
    "struct DirectionalLight"
    "{"
    " vec3 color;"
    " vec3 direction;"

    " float intensityAmbient;"
    " float intensityDiffuse;"
    " float intensitySpecular;"
    "};"

    "\n#define MAX_POINT_LIGHTS 1\n"
    "struct PointLight"
    "{"
    " vec3 color;"
    " vec3 position;"

    " float attenuationLinear;"
    " float attenuationQuadratic;"

    " float intensityAmbient;"
    " float intensityDiffuse;"
    " float intensitySpecular;"
    "};"

    "out vec4 fragmentColor;"
    "in vec2 textureCoords;"

    "uniform sampler2D gPosition;"
    "uniform sampler2D gNormal;"
    "uniform sampler2D gColor;"
    "uniform vec3 viewPosition;"

    "uniform DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];"
    "uniform PointLight pointLights[MAX_POINT_LIGHTS];"

    "vec3 computeDirectionalLightContribution(DirectionalLight directionalLight, vec3 objectColor, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)"
    "{"
    " vec3 lightDirection = normalize(-directionalLight.direction);"
    " vec3 reflectDirection = reflect(-lightDirection, normal);"

    " vec3 ambient = directionalLight.intensityAmbient * objectColor;"
    " vec3 diffuse = directionalLight.intensityDiffuse * directionalLight.color *"
    "                max(dot(normal, lightDirection), 0.f) * objectColor;"
    " vec3 specular = directionalLight.intensitySpecular * directionalLight.color *"
    "                 pow(max(dot(viewDirection, reflectDirection), 0.f), 64);" //no specular map yet

    " return (ambient + diffuse + specular);"
    "}"

    "vec3 computePointLightContribution(PointLight pointLight, vec3 objectColor, vec3 normal, vec3 fragmentPosition, vec3 viewDirection)"
    "{"
    " vec3 lightDirection = normalize(pointLight.position - fragmentPosition);"
    " vec3 reflectDirection = reflect(-lightDirection, normal);"

    " vec3 ambient = pointLight.intensityAmbient * objectColor;"
    " vec3 diffuse = pointLight.intensityDiffuse * pointLight.color *"
    "                max(dot(normal, lightDirection), 0.f) * objectColor;"
    " vec3 specular = pointLight.intensitySpecular * pointLight.color *"
    "                 pow(max(dot(viewDirection, reflectDirection), 0.f), 64);" //no specular map yet

    " float distance = length(pointLight.position - fragmentPosition);"
    " float attenuation = 1.f / (1.f + "
    "                            pointLight.attenuationLinear * distance +"
    "                            pointLight.attenuationQuadratic * distance * distance);"

    " return (ambient + diffuse + specular) * attenuation;"
    "}"

    //This rand function is copy/pasted from interwebs.
    //http://byteblacksmith.com/improvements-to-the-canonical-one-liner-glsl-rand-for-opengl-es-2-0/
    "highp float rand(vec2 co)"
    "{"
    " highp float a = 12.9898;"
    " highp float b = 78.233;"
    " highp float c = 43758.5453;"
    " highp float dt= dot(co.xy ,vec2(a,b));"
    " highp float sn= mod(dt,3.14);"
    " return fract(sin(sn) * c);"
    "}"

    "void main()"
    "{"
    " vec3 normal = texture2D(gNormal, textureCoords).rgb;"
    " if(normal == vec3(0.f))"
    "  fragmentColor = vec4(.4f, .6f, .2f, 1.f);"
    " else"
    " {"
    "  vec3 objectColor = texture2D(gColor, textureCoords).rgb;"
    "  vec3 fragmentPosition = texture2D(gPosition, textureCoords).rgb;"
    "  vec3 viewDirection = normalize(viewPosition - fragmentPosition);"
    "  vec3 result = .1f * objectColor;"
    "  for(int DirectionalLightIndex = 0; DirectionalLightIndex < MAX_DIRECTIONAL_LIGHTS; ++DirectionalLightIndex)"
    "  {"
    "   result += computeDirectionalLightContribution(directionalLights[DirectionalLightIndex], objectColor, normal, fragmentPosition, viewDirection);"
    "  }"
    "  for(int PointLightIndex = 0; PointLightIndex < MAX_POINT_LIGHTS; ++PointLightIndex)"
    "  {"
    "   result += computePointLightContribution(pointLights[PointLightIndex], objectColor, normal, fragmentPosition, viewDirection);"
    "  }"
    "  fragmentColor = vec4(result + mix(-0.5f/255.f, 0.5f/255.f, rand(textureCoords)), 1.f);"
    " }"
    "}";

  GLchar* guiVertexShaderSource =
    "#version 400 core\n"
    "out VS_OUT"
    "{"
    " float t;"
    " float barWidth;"
    " vec2 dimensions;"
    "} vs_out;"

    "layout (location = 0) in vec2 position;"
    "layout (location = 1) in vec2 dimensions;"
    "layout (location = 2) in float barWidth;"
    "layout (location = 3) in float t;"

    "void main()"
    "{"
    " gl_Position = vec4(position.x, position.y, 0.0f, 1.0f);"
    " vs_out.t = t;"
    " vs_out.dimensions = dimensions;"
    " vs_out.barWidth = barWidth;"
    "}";

  GLchar* guiGeometryShaderSource =
    "#version 400 core\n"
    "in VS_OUT"
    "{"
    " float t;"
    " float barWidth;"
    " vec2 dimensions;"
    "} gs_in[];"

    "layout (points) in;"
    "layout (triangle_strip, max_vertices = 12) out;"

    "uniform float onePixel;"

    "void buildSquare(vec4 position, float width, float height)"
    "{"
    " gl_Position = position;"
    " EmitVertex();"
    " gl_Position = position + vec4(width, 0.f, 0.f, 0.f);"
    " EmitVertex();"
    " gl_Position = position + vec4(0.f, -height, 0.f, 0.f);"
    " EmitVertex();"
    " gl_Position = position + vec4(width, -height, 0.f, 0.f);"
    " EmitVertex();"
    " EndPrimitive();"
    "}"
    "void main()"
    "{"
    " float width = gs_in[0].dimensions.x;"
    " float barWidth = gs_in[0].barWidth;"
    " float t = gs_in[0].t * (width - barWidth - onePixel * 2);"
    " float height = gs_in[0].dimensions.y;"
    " buildSquare(gl_in[0].gl_Position, t, height);"
    " buildSquare(gl_in[0].gl_Position + vec4(t + onePixel, vec3(0.f)), barWidth, height);"
    " buildSquare(gl_in[0].gl_Position + vec4(t + onePixel * 2 + barWidth, vec3(0.f)), width - t - onePixel * 2 - barWidth, height);"
    "}";

  GLchar* guiFragmentShaderSource =
    "#version 400 core\n"
    "out vec4 fragmentColor;"
    "void main()"
    "{"
    " fragmentColor = vec4(1.f);"
    "}";

  ShaderObject gPassShader = {};
  gPassShader.vertexShaderSource = gPassVertexShaderSource;
  gPassShader.fragmentShaderSource = gPassFragmentShaderSource;
  compileShaderObject(&gPassShader);
  linkShaderObject(&gPassShader);

  glUseProgram(gPassShader.shaderProgramID);
  GLuint perlinNoiseLocation = glGetUniformLocation(gPassShader.shaderProgramID, "perlinNoise");
  glUniform1i(perlinNoiseLocation, 0);

  ShaderObject lPassShader = {};
  lPassShader.vertexShaderSource = lPassVertexShaderSource;
  lPassShader.fragmentShaderSource = lPassFragmentShaderSource;
  compileShaderObject(&lPassShader);
  linkShaderObject(&lPassShader);

  glUseProgram(lPassShader.shaderProgramID);
  GLuint gPositionLocation = glGetUniformLocation(lPassShader.shaderProgramID, "gPosition");
  glUniform1i(gPositionLocation, 0);
  GLuint gNormalLocation = glGetUniformLocation(lPassShader.shaderProgramID, "gNormal");
  glUniform1i(gNormalLocation, 1);
  GLuint gColorLocation = glGetUniformLocation(lPassShader.shaderProgramID, "gColor");
  glUniform1i(gColorLocation, 2);

  ShaderObject guiShader = {};
  guiShader.vertexShaderSource = guiVertexShaderSource;
  guiShader.geometryShaderSource = guiGeometryShaderSource;
  guiShader.fragmentShaderSource = guiFragmentShaderSource;
  compileShaderObject(&guiShader);
  linkShaderObject(&guiShader);

  GLuint gBuffer;
  glGenFramebuffers(1, &gBuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
  GLuint gPosition, gNormal, gColor;

  glGenTextures(1, &gPosition);
  glBindTexture(GL_TEXTURE_2D, gPosition);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         gPosition, 0);

  glGenTextures(1, &gNormal);
  glBindTexture(GL_TEXTURE_2D, gNormal);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         gNormal, 0);

  glGenTextures(1, &gColor);
  glBindTexture(GL_TEXTURE_2D, gColor);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         gColor, 0);

  GLuint attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
  glDrawBuffers(3, attachments);

  GLuint rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rboDepth);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Framebuffer is broken!");

  GLuint quadVAO;
  GLuint quadVBO;
  GLfloat quadVertices[] = {
    -1.f, 1.f, 0.f,     0.f, 1.f,
    -1.f, -1.f, 0.f,    0.f, 0.f,
    1.f, 1.f, 0.f,      1.f, 1.f,
    1.f, -1.f, 0.f,      1.f, 0.f
  };

  glGenVertexArrays(1, &quadVAO);
  glGenBuffers(1, &quadVBO);
  glBindVertexArray(quadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));

#define P2F(X, DIM) ((X)/(float)(DIM)) * 2.f - 1.f
  float sliderStaticInfo[] = {
    P2F(0, SCREEN_WIDTH), P2F(SCREEN_HEIGHT, SCREEN_HEIGHT),
    200.f / SCREEN_WIDTH * 2, 5.f / SCREEN_HEIGHT * 2,
    5.f / SCREEN_WIDTH * 2,

    P2F(0, SCREEN_WIDTH), P2F(SCREEN_HEIGHT - 6, SCREEN_HEIGHT),
    200.f / SCREEN_WIDTH * 2, 5.f / SCREEN_HEIGHT * 2,
    5.f / SCREEN_WIDTH * 2,

    P2F(0, SCREEN_WIDTH), P2F(SCREEN_HEIGHT - 12, SCREEN_HEIGHT),
    200.f / SCREEN_WIDTH * 2, 5.f / SCREEN_HEIGHT * 2,
    5.f / SCREEN_WIDTH * 2,


    P2F(0, SCREEN_WIDTH), P2F(SCREEN_HEIGHT - 24, SCREEN_HEIGHT),
    200.f / SCREEN_WIDTH * 2, 5.f / SCREEN_HEIGHT * 2,
    20.f / SCREEN_WIDTH * 2,

    P2F(0, SCREEN_WIDTH), P2F(SCREEN_HEIGHT - 30, SCREEN_HEIGHT),
    200.f / SCREEN_WIDTH * 2, 5.f / SCREEN_HEIGHT * 2,
    20.f / SCREEN_WIDTH * 2
  };

  float sliderTs[5] = {};

  SliderGUI sliderGUI = {};
  initSliderGUI(&sliderGUI, sliderStaticInfo, sliderTs, 5);

  GLuint perlinNoiseTextureID;
  glGenTextures(1, &perlinNoiseTextureID);
  generate2DPerlinNoise(perlinNoiseTextureID, 420);

  Camera camera;
  initCamera(&camera, (GLfloat)width, (GLfloat)height);

  DirectionalLight directionalLights[1] = {};
  directionalLights[0].direction = glm::vec3(0.f, 5.f, -2.f);
  directionalLights[0].color = glm::vec3(0.f, 1.f, 0.f);
  directionalLights[0].intensity.ambient = 255 * 0.1f;
  directionalLights[0].intensity.diffuse = 255 * 0.5f;
  directionalLights[0].intensity.specular = 255 * 0.5f;

  PointLight pointLights[1] = {};
  pointLights[0].position = glm::vec3(0.f, 5.f, -2.f);
  pointLights[0].color = glm::vec3(1.f, 0.1f, 0.1f);
  RenderObject lightShape;
  initRenderObject(&lightShape,
                   par_shapes_create_parametric_sphere(32, 32));
  lightShape.model = glm::scale(lightShape.model, glm::vec3(0.5f, 0.5f, 0.5f));
  lightShape.color = pointLights[0].color;

  RenderObject shape;
  initRenderObject(&shape,
                   par_shapes_create_parametric_sphere(32, 32));
  shape.model = glm::translate(shape.model, glm::vec3(0.f, 0.f, 0.f));

  RenderObject shape2;
  initRenderObject(&shape2,
                   par_shapes_create_parametric_sphere(32, 32));
  shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 1.f, -3.f));
  shape2.color = glm::vec3(.2f, 1.f, 1.f);

  float* currentSlider = &pointLights[0].intensity.ambient;

  GLfloat mouseCoordsAtLastFrameStart[2];
  mouseCoordsAtLastFrameStart[0] = mouseCoords[0];
  mouseCoordsAtLastFrameStart[1] = mouseCoords[1];

  GLfloat dT = 0.f;
  GLfloat timeAtLastFrameStart = 0.f;
  while(!glfwWindowShouldClose(window))
  {
    GLfloat timeNow = (GLfloat)glfwGetTime();
    dT = timeNow - timeAtLastFrameStart;

    timeAtLastFrameStart = timeNow;

    glfwPollEvents();

    GLfloat mouseCoordsNow[2];
    mouseCoordsNow[0] = mouseCoords[0];
    mouseCoordsNow[1] = mouseCoords[1];

    updateCamera(&camera,
                 mouseCoordsNow[0] - mouseCoordsAtLastFrameStart[0],
                 mouseCoordsNow[1] - mouseCoordsAtLastFrameStart[1],
                 keys,
                 dT);

    mouseCoordsAtLastFrameStart[0] = mouseCoordsNow[0];
    mouseCoordsAtLastFrameStart[1] = mouseCoordsNow[1];

#define SLIDERSPEED 25
    if(keys[GLFW_KEY_1])
      currentSlider = &pointLights[0].intensity.ambient;
    else if(keys[GLFW_KEY_2])
      currentSlider = &pointLights[0].intensity.diffuse;
    else if(keys[GLFW_KEY_3])
      currentSlider = &pointLights[0].intensity.specular;
    else if(keys[GLFW_KEY_4])
      currentSlider = &pointLights[0].attenuation.linear;
    else if(keys[GLFW_KEY_5])
      currentSlider = &pointLights[0].attenuation.quadratic;

    if(keys[GLFW_KEY_Q])
      *currentSlider -= SLIDERSPEED * dT;
    if(keys[GLFW_KEY_E])
      *currentSlider += SLIDERSPEED * dT;

    if(*currentSlider < 0)
      *currentSlider = 0;
    if(*currentSlider > 255)
      *currentSlider = 255;

#define ROTATION_SPEED .2f
    shape.model = glm::rotate(shape.model, ROTATION_SPEED * dT, glm::vec3(0.5f, .5f, 0.5f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(0.3f, 0.f, 0.6f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, 5.f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(1.f, 0.f, 0.f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, -5.f));

    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(gPassShader.shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, perlinNoiseTextureID);

    GLint projectionLocation = glGetUniformLocation(gPassShader.shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera.projection));
    GLint viewLocation = glGetUniformLocation(gPassShader.shaderProgramID, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.view));

    drawRenderObject(&shape2, gPassShader.shaderProgramID);
    drawRenderObject(&shape, gPassShader.shaderProgramID);
    drawPointLightDebugModel(lightShape, &pointLights[0], gPassShader.shaderProgramID);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(lPassShader.shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gColor);

    GLint viewPositionLocation = glGetUniformLocation(lPassShader.shaderProgramID, "viewPosition");
    glUniform3fv(viewPositionLocation, 1, glm::value_ptr(camera.position));

    {
      GLint lightDirectionLocation= glGetUniformLocation(lPassShader.shaderProgramID, "directionalLights[0].direction");
      glUniform3fv(lightDirectionLocation, 1, glm::value_ptr(directionalLights[0].direction));
      GLint lightColorLocation = glGetUniformLocation(lPassShader.shaderProgramID, "directionalLights[0].color");
      glUniform3fv(lightColorLocation, 1, glm::value_ptr(directionalLights[0].color));

      GLint ambientIntensityLocation = glGetUniformLocation(lPassShader.shaderProgramID, "directionalLights[0].intensityAmbient");
      glUniform1f(ambientIntensityLocation, directionalLights[0].intensity.ambient / 255.f);
      GLint diffuseIntensityLocation = glGetUniformLocation(lPassShader.shaderProgramID, "directionalLights[0].intensityDiffuse");
      glUniform1f(diffuseIntensityLocation, directionalLights[0].intensity.diffuse / 255.f);
      GLint specularIntensityLocation = glGetUniformLocation(lPassShader.shaderProgramID, "directionalLights[0].intensitySpecular");
      glUniform1f(specularIntensityLocation, directionalLights[0].intensity.specular / 255.f);
    }

    {
      GLint lightPositionLocation= glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].position");
      glUniform3fv(lightPositionLocation, 1, glm::value_ptr(pointLights[0].position));
      GLint lightColorLocation = glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].color");
      glUniform3fv(lightColorLocation, 1, glm::value_ptr(pointLights[0].color));

      GLint ambientIntensityLocation = glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].intensityAmbient");
      glUniform1f(ambientIntensityLocation, pointLights[0].intensity.ambient / 255.f);
      GLint diffuseIntensityLocation = glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].intensityDiffuse");
      glUniform1f(diffuseIntensityLocation, pointLights[0].intensity.diffuse / 255.f);
      GLint specularIntensityLocation = glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].intensitySpecular");
      glUniform1f(specularIntensityLocation, pointLights[0].intensity.specular / 255.f);

      GLint linearAttenuationLocation = glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].attenuationLinear");
      glUniform1f(linearAttenuationLocation, pointLights[0].attenuation.linear / 255.f);
      GLint quadraticAttenuationLocation = glGetUniformLocation(lPassShader.shaderProgramID, "pointLights[0].attenuationQuadratic");
      glUniform1f(quadraticAttenuationLocation, pointLights[0].attenuation.quadratic / 255.f);
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(guiShader.shaderProgramID);

    GLint onePixelLocation = glGetUniformLocation(guiShader.shaderProgramID, "onePixel");
    glUniform1f(onePixelLocation, 1.f / SCREEN_WIDTH * 2);

    sliderTs[0] = pointLights[0].intensity.ambient / 255.f;
    sliderTs[1] = pointLights[0].intensity.diffuse / 255.f;
    sliderTs[2] = pointLights[0].intensity.specular / 255.f;
    sliderTs[3] = pointLights[0].attenuation.linear / 255.f;
    sliderTs[4] = pointLights[0].attenuation.quadratic / 255.f;

    glBindBuffer(GL_ARRAY_BUFFER, sliderGUI.vboTs);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * 5,
                 sliderTs,
                 GL_DYNAMIC_DRAW);

    glBindVertexArray(sliderGUI.VAO);
    glDrawArrays(GL_POINTS, 0, 5);
    glBindVertexArray(0);

    GLenum err;
    bool shouldQuit = false;
    while((err = glGetError()) != GL_NO_ERROR)
    {
      printf("Error: %i\n", err);
      shouldQuit = true;
    }
    if(shouldQuit)
    {
      glfwSetWindowShouldClose(window, GL_TRUE);
    }

    glfwSwapBuffers(window);
  }

  par_shapes_free_mesh(shape.mesh);
  destroyRenderObject(&shape);

  par_shapes_free_mesh(shape2.mesh);
  destroyRenderObject(&shape2);

  glfwTerminate();

	return 0;
}
