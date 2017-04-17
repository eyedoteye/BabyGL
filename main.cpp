#include <stdio.h>
//Todo: Use define to quick swap between windows mode and console mode.
#include <windows.h>

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
#include "PerlinTexture.cpp"

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
  union
  {
    struct
    {
      float ambient;
      float diffuse;
      float specular;
    } intensity;
    float intensities[3];
  };
  union
  {
    struct
    {
      float linear;
      float quadratic;
    } attenuation;
    float attenuations[2];
  };
};

void flattenPointLightSliders(
  PointLight* pointLight,
  float** lightSlidersArray)
{
  for(int intensityIndex = 0;
      intensityIndex < 3;
      ++intensityIndex)
  {
    lightSlidersArray[intensityIndex] =
      &pointLight->intensities[intensityIndex];
  }
  for(int attenuationIndex = 0;
      attenuationIndex < 2;
      ++attenuationIndex)
  {
    lightSlidersArray[3 + attenuationIndex] =
      &pointLight->attenuations[attenuationIndex];
  }
}

void drawPointLightDebugModel(
  RenderObject debugModel,
  PointLight* pointLight,
  float scale,
  GLuint shaderProgramID)
{
  debugModel.model = glm::translate(debugModel.model, pointLight->position);
  debugModel.model = glm::scale(debugModel.model, glm::vec3(scale, scale, scale));
  debugModel.color = pointLight->color;
  drawRenderObject(&debugModel, shaderProgramID);
}

void drawPointLightDebugModel(
  RenderObject debugModel,
  PointLight* pointLight,
  GLuint shaderProgramID)
{
  debugModel.model = glm::translate(debugModel.model, pointLight->position);
  debugModel.color = pointLight->color;
  drawRenderObject(&debugModel, shaderProgramID);
}

struct SliderGUI
{
  GLuint VAO;
  GLuint vboPositions, vboTs, vboIsSelecteds;

  int count;
  float* positions;
  float* ts;
  int* isSelecteds;
};

void initSliderGUI(SliderGUI *sliderGUI, int count)
{
  glGenVertexArrays(1, &sliderGUI->VAO);
  glGenBuffers(1, &sliderGUI->vboPositions);
  glGenBuffers(1, &sliderGUI->vboTs);
  glGenBuffers(1, &sliderGUI->vboIsSelecteds);

  glBindVertexArray(sliderGUI->VAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, sliderGUI->vboPositions);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * count * 5,
                 sliderGUI->positions,
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

    glBindBuffer(GL_ARRAY_BUFFER, sliderGUI->vboTs);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * count,
                 sliderGUI->ts,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
                          sizeof(float),
                          (GLvoid*)0);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, sliderGUI->vboIsSelecteds);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(int) * count,
                 sliderGUI->isSelecteds,
                 GL_DYNAMIC_DRAW);
    glVertexAttribPointer(4, 1, GL_INT, GL_FALSE,
                          sizeof(int),
                          (GLvoid*)0);
    glEnableVertexAttribArray(4);
  }
}

#pragma warning(push)
#pragma warning(disable:4100)
static bool keysPressed[1024];
static bool keysHeld[1024];
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
  if(key == GLFW_KEY_ESCAPE)
  {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if(action == GLFW_PRESS)
  {
    keysPressed[key] = true;
    keysHeld[key] = true; 
  }
  else if(action == GLFW_RELEASE)
  {
    keysHeld[key] = false;
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
//int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
//    PSTR lpCmdLine, INT nCmdShow)
int main()
{
  bool debugMode = false;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Duckbut",
                                        NULL, NULL);
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
    #include "gPass.vs"
  GLchar* gPassFragmentShaderSource =
    #include "gPass.fs"

  GLchar* lPassVertexShaderSource =
    #include "lPass.vs"
  GLchar* lPassFragmentShaderSource =
    #include "lPass.fs"

  GLchar* debugObjectsVertexShaderSource =
    #include "debugObjects.vs"
  GLchar* debugObjectsFragmentShaderSource =
    #include "debugObjects.fs"
  GLchar* pointLightOutlineVertexShaderSource =
    #include "pointLightOutline.vs"
  GLchar* pointLightOutlineFragmentShaderSource =
    #include "pointLightOutline.fs"
  
  GLchar* guiVertexShaderSource =
    #include "gui.vs"
  GLchar* guiGeometryShaderSource =
    #include "gui.gs"
  GLchar* guiFragmentShaderSource =
    #include "gui.fs"

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

  GLuint positionBufferLocation = glGetUniformLocation(
    lPassShader.shaderProgramID, "positionBuffer");
  glUniform1i(positionBufferLocation, 0);
  GLuint normalBufferLocation = glGetUniformLocation(
    lPassShader.shaderProgramID, "normalBuffer");
  glUniform1i(normalBufferLocation, 1);
  GLuint colorBufferLocation = glGetUniformLocation(
    lPassShader.shaderProgramID, "colorBuffer");
  glUniform1i(colorBufferLocation, 2);

  GLuint debugOnSubroutineIndex = glGetSubroutineIndex(
    lPassShader.shaderProgramID,
    GL_FRAGMENT_SHADER, "debugOn");
  GLuint debugOffSubroutineIndex = glGetSubroutineIndex(
    lPassShader.shaderProgramID,
    GL_FRAGMENT_SHADER, "debugOff");

  glUseProgram(0);

  ShaderObject debugObjectsShader = {};
  debugObjectsShader.vertexShaderSource = debugObjectsVertexShaderSource;
  debugObjectsShader.fragmentShaderSource = debugObjectsFragmentShaderSource;
  compileShaderObject(&debugObjectsShader);
  linkShaderObject(&debugObjectsShader);
 
  glUseProgram(debugObjectsShader.shaderProgramID);

  GLuint discardNonSpheresIndex = glGetSubroutineIndex(
    debugObjectsShader.shaderProgramID,
    GL_FRAGMENT_SHADER, "discardNonSpheres");

  GLuint blurIndices[2];
  blurIndices[0] = glGetSubroutineIndex(
    debugObjectsShader.shaderProgramID,
    GL_FRAGMENT_SHADER, "blurHorizontal");
  blurIndices[1] = glGetSubroutineIndex(
    debugObjectsShader.shaderProgramID,
    GL_FRAGMENT_SHADER, "blurVertical");
  
  glUseProgram(0);

  ShaderObject pointLightOutlineShader = {};
  pointLightOutlineShader.vertexShaderSource = pointLightOutlineVertexShaderSource;
  pointLightOutlineShader.fragmentShaderSource = pointLightOutlineFragmentShaderSource;
  compileShaderObject(&pointLightOutlineShader);
  linkShaderObject(&pointLightOutlineShader);

  ShaderObject guiShader = {};
  guiShader.vertexShaderSource = guiVertexShaderSource;
  guiShader.geometryShaderSource = guiGeometryShaderSource;
  guiShader.fragmentShaderSource = guiFragmentShaderSource;
  compileShaderObject(&guiShader);
  linkShaderObject(&guiShader);

  GLuint gBufferID;
  glGenFramebuffers(1, &gBufferID);
  glBindFramebuffer(GL_FRAMEBUFFER, gBufferID);
  
  // Note: gBuffer textures for defaulty shaded objects. 
  GLuint positionBufferID, normalBufferID, colorBufferID;

  glGenTextures(1, &positionBufferID);
  glBindTexture(GL_TEXTURE_2D, positionBufferID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         positionBufferID, 0);

  glGenTextures(1, &normalBufferID);
  glBindTexture(GL_TEXTURE_2D, normalBufferID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
                         normalBufferID, 0);

  glGenTextures(1, &colorBufferID);
  glBindTexture(GL_TEXTURE_2D, colorBufferID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
                         colorBufferID, 0);
  
  // Note: gBuffer textures for point lights.
  GLuint pointLightColorBufferID;

  glGenTextures(1, &pointLightColorBufferID);
  glBindTexture(GL_TEXTURE_2D, pointLightColorBufferID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
                         pointLightColorBufferID, 0);

  GLuint attachments[4] = {
    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
    GL_COLOR_ATTACHMENT3 };
  glDrawBuffers(4, attachments);

  GLuint rboDepth;
  glGenRenderbuffers(1, &rboDepth);
  glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rboDepth);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Framebuffer is broken!");
  
  GLuint pointLightFBOs[2];
  glGenFramebuffers(2, pointLightFBOs);

  GLuint pointLightBlurBufferIDs[2];
  glGenTextures(2, pointLightBlurBufferIDs);
  for(int pointLightBlurBufferIndex = 0;
      pointLightBlurBufferIndex < 2;
      ++pointLightBlurBufferIndex)
  {
    glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBOs[pointLightBlurBufferIndex]);
    glBindTexture(GL_TEXTURE_2D, pointLightBlurBufferIDs[pointLightBlurBufferIndex]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0,
                 GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, pointLightBlurBufferIDs[pointLightBlurBufferIndex], 0);
    GLuint attachment[1] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, attachment);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      printf("Framebuffer is broken!");
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  //Note: framebuffer for outline
  GLuint pointLightOutlineFBO;
  glGenFramebuffers(1, &pointLightOutlineFBO);

  GLuint pointLightOutlineColorBufferID;
  glGenTextures(1, &pointLightOutlineColorBufferID);

  glBindFramebuffer(GL_FRAMEBUFFER, pointLightOutlineFBO);
  glBindTexture(GL_TEXTURE_2D, pointLightOutlineColorBufferID);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0,
               GL_RGB, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D, pointLightOutlineColorBufferID, 0);
  
  GLuint pointLightOutlineStencilBufferID;
  glGenRenderbuffers(1, &pointLightOutlineStencilBufferID);
  glBindRenderbuffer(GL_RENDERBUFFER, pointLightOutlineStencilBufferID);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                            GL_RENDERBUFFER, pointLightOutlineStencilBufferID);
  if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    printf("Framebuffer is broken!");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  
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
  int sliderIsSelecteds[5] = {};

  SliderGUI sliderGUI = {};
  sliderGUI.positions = sliderStaticInfo;
  sliderGUI.ts = sliderTs;
  sliderGUI.isSelecteds = sliderIsSelecteds;
  initSliderGUI(&sliderGUI, 5);

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

#define POINT_LIGHT_COUNT 2
  PointLight pointLights[POINT_LIGHT_COUNT] = {};
  pointLights[0].position = glm::vec3(0.f, 5.f, -2.f);
  pointLights[0].color = glm::vec3(1.f, 0.1f, 0.1f);
  pointLights[1].position = glm::vec3(0.f, 1.f, -8.f);
  pointLights[1].color = glm::vec3(1.f, 1.f, 1.f);

  GLuint pointLightsUBOIndex = glGetUniformBlockIndex(
      lPassShader.shaderProgramID,
      "pointLightsUBO");
  glUniformBlockBinding(
      lPassShader.shaderProgramID,
      pointLightsUBOIndex, 0);
  
  GLuint pointLightsUBO;
  glGenBuffers(1, &pointLightsUBO);
  int pointLightsUBOSize = sizeof(float) * 4 //vec3 color (vec4 min)
                         + sizeof(float) * 4 //vec3 position (vec4 min)
                         + sizeof(float)     //float attenuationLinear
                         + sizeof(float)     //float attenuationQuadratic
                         + sizeof(float)     //float intensityAmbient
                         + sizeof(float)     //float intensityDiffuse
                         + sizeof(float);    //float intensitySpecular
  int alignment = sizeof(float) * 4;  //(vec4 min for structs)
  pointLightsUBOSize = (pointLightsUBOSize + (alignment - 1))
                     / alignment * alignment;
  int totalsize = pointLightsUBOSize * POINT_LIGHT_COUNT;

  glBindBuffer(GL_UNIFORM_BUFFER, pointLightsUBO);
  glBufferData(GL_UNIFORM_BUFFER, totalsize, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, pointLightsUBO, 0, totalsize);

#define OBJECT_TYPE_NONE 0
#define OBJECT_TYPE_DEFAULT 1
#define OBJECT_TYPE_POINTLIGHT 2
  RenderObject lightShape;
  lightShape.objectType = OBJECT_TYPE_POINTLIGHT;
  lightShape.mesh = par_shapes_create_parametric_sphere(32, 32);
  lightShape.model = glm::scale(lightShape.model, glm::vec3(0.5f, 0.5f, 0.5f));
  initRenderObject(&lightShape);  

  RenderObject shape;
  shape.objectType = OBJECT_TYPE_DEFAULT;
  shape.mesh = par_shapes_create_parametric_sphere(32, 32);
  shape.model = glm::translate(shape.model, glm::vec3(0.f, 0.f, 0.f));
  shape.color = glm::vec3(0.7f); 
  initRenderObject(&shape);

  RenderObject shape2;
  shape2.objectType = OBJECT_TYPE_DEFAULT;
  shape2.mesh = par_shapes_create_parametric_sphere(32, 32);
  shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 1.f, -3.f));
  shape2.color = glm::vec3(.2f, 1.f, 1.f);
  initRenderObject(&shape2);

  char currentPointLightIndex = 0;
  char currentSliderIndex = 0;
  float* currentLightSliders[5] = {};
  flattenPointLightSliders(&pointLights[currentPointLightIndex],
                           currentLightSliders);
    
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

    memset(keysPressed, 0, 1024 * sizeof(bool)); 

    glfwPollEvents();

    GLfloat mouseCoordsNow[2];
    mouseCoordsNow[0] = mouseCoords[0];
    mouseCoordsNow[1] = mouseCoords[1];

    updateCamera(&camera,
                 mouseCoordsNow[0] - mouseCoordsAtLastFrameStart[0],
                 mouseCoordsNow[1] - mouseCoordsAtLastFrameStart[1],
                 keysHeld,
                 dT);

    mouseCoordsAtLastFrameStart[0] = mouseCoordsNow[0];
    mouseCoordsAtLastFrameStart[1] = mouseCoordsNow[1];

#define SLIDERSPEED (25 / 255.f)
    if(keysPressed[GLFW_KEY_TAB])
    {
      if(keysPressed[GLFW_MOD_SHIFT])
        --currentPointLightIndex;     
      else
        ++currentPointLightIndex;
      
      currentPointLightIndex = (currentPointLightIndex + POINT_LIGHT_COUNT)
                                  % POINT_LIGHT_COUNT;
      flattenPointLightSliders(&pointLights[currentPointLightIndex],
                               currentLightSliders);
    }

    if(keysPressed[GLFW_KEY_GRAVE_ACCENT])
      debugMode = !debugMode;

    if(debugMode)
    {
      if(keysPressed[GLFW_KEY_1])
        currentSliderIndex = 0;
      else if(keysPressed[GLFW_KEY_2])
        currentSliderIndex = 1;
      else if(keysPressed[GLFW_KEY_3])
        currentSliderIndex = 2;
      else if(keysPressed[GLFW_KEY_4])
        currentSliderIndex = 3;
      else if(keysPressed[GLFW_KEY_5])
        currentSliderIndex = 4;

      float* currentLightSlider = currentLightSliders[currentSliderIndex];
      if(keysHeld[GLFW_KEY_Q])
        *currentLightSlider -= SLIDERSPEED * dT;
      if(keysHeld[GLFW_KEY_E])
        *currentLightSlider += SLIDERSPEED * dT;

      if(*currentLightSlider < 0)
        *currentLightSlider = 0;
      if(*currentLightSlider > 1.f)
        *currentLightSlider = 1;
    }

#define ROTATION_SPEED .2f
    shape.model = glm::rotate(shape.model, ROTATION_SPEED * dT, glm::vec3(0.5f, .5f, 0.5f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(0.3f, 0.f, 0.6f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, 5.f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(1.f, 0.f, 0.f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, -5.f));
    glBindFramebuffer(GL_FRAMEBUFFER, gBufferID);
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//    glClearFramebufferfv(GL_COLOR, normalBufferID);
    glUseProgram(gPassShader.shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, perlinNoiseTextureID);

    GLint projectionLocation = glGetUniformLocation(gPassShader.shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera.projection));
    GLint viewLocation = glGetUniformLocation(gPassShader.shaderProgramID, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.view));

    drawRenderObject(&shape2, gPassShader.shaderProgramID);
    drawRenderObject(&shape, gPassShader.shaderProgramID);
    if(debugMode)
    {
      for(int pointLightIndex = 0; pointLightIndex < POINT_LIGHT_COUNT; ++pointLightIndex)
      {
        drawPointLightDebugModel(
          lightShape, 
          &pointLights[pointLightIndex],
          gPassShader.shaderProgramID);
      }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(.4f, .6f, .2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(lPassShader.shaderProgramID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, positionBufferID);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, normalBufferID);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, colorBufferID);

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
      glBindBuffer(GL_UNIFORM_BUFFER, pointLightsUBO);
      float buffer[26];
      for(int pointLightIndex = 0;
          pointLightIndex < POINT_LIGHT_COUNT;
          ++pointLightIndex)
      {
        int offset = pointLightIndex * 13;
        int size = sizeof(float) * 4; //vec3 color (vec4 min)
       memcpy(buffer + offset,
           glm::value_ptr(pointLights[pointLightIndex].color),
           size);

        offset += 4;
        //vec3 position (vec4 min)
        memcpy(buffer + offset,
          glm::value_ptr(pointLights[pointLightIndex].position),
          size);

        offset += 3;
        size = sizeof(float) * 2; //float attenuations[2]
        memcpy(buffer + offset,
          pointLights[pointLightIndex].attenuations,
          size);

        offset += 2;
        size = sizeof(float) * 3; //float intensities[3]
        memcpy(buffer + offset,
          pointLights[pointLightIndex].intensities,
          size);
      }
      glBufferSubData(GL_UNIFORM_BUFFER,
        0, sizeof(buffer), buffer);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);
    }    

    if(debugMode) //The bug is in here
    {
      glUseProgram(debugObjectsShader.shaderProgramID);
      
      //Note: Now I'm Just str8 Copying LearnOpenGL
      GLboolean vertical = false;
      for(int iteration = 0; iteration < 10; ++iteration)
      {
        glBindFramebuffer(GL_FRAMEBUFFER, pointLightFBOs[vertical]);
        glActiveTexture(GL_TEXTURE0);
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &blurIndices[vertical]);
        glBindTexture(GL_TEXTURE_2D,
           iteration == 0 ? pointLightColorBufferID : pointLightBlurBufferIDs[!vertical]);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        vertical = !vertical;
      }

      glUseProgram(lPassShader.shaderProgramID);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &debugOnSubroutineIndex);
      glActiveTexture(GL_TEXTURE3);
      glBindTexture(GL_TEXTURE_2D, pointLightBlurBufferIDs[!vertical]);
    }
    else
    {
      glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1, &debugOffSubroutineIndex);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    if(debugMode)
    {
      //Todo: Put the stencil in here 
      glEnable(GL_STENCIL_TEST);
      glBindFramebuffer(GL_FRAMEBUFFER, pointLightOutlineFBO);
      glUseProgram(pointLightOutlineShader.shaderProgramID);
      projectionLocation = glGetUniformLocation(pointLightOutlineShader.shaderProgramID, "projection");
      glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera.projection));
      viewLocation = glGetUniformLocation(pointLightOutlineShader.shaderProgramID, "view");
      glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.view));
      glStencilFunc(GL_ALWAYS, 1, 0xFF);
      glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
      glStencilMask(0xFF);
      glDepthMask(GL_FALSE);
      glClearColor(1.f, 1.f, 1.f, 1.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      drawPointLightDebugModel(
          lightShape,
          &pointLights[currentPointLightIndex],
          0.8f,
          pointLightOutlineShader.shaderProgramID); 
      glBindFramebuffer(GL_READ_FRAMEBUFFER, pointLightOutlineFBO);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      glBlitFramebuffer(0, 0, width, height,
                        0, 0, width, height,
                        GL_STENCIL_BUFFER_BIT, GL_NEAREST);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
      glStencilMask(0x00);
      glDepthMask(GL_TRUE);
      glClear(GL_DEPTH_BUFFER_BIT);
      drawPointLightDebugModel(
          lightShape,
          &pointLights[currentPointLightIndex],
          pointLightOutlineShader.shaderProgramID);
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
      glDisable(GL_STENCIL_TEST);
      glUseProgram(guiShader.shaderProgramID);

      GLint onePixelLocation = glGetUniformLocation(guiShader.shaderProgramID, "onePixel");
      glUniform1f(onePixelLocation, 1.f / SCREEN_WIDTH * 2);

      sliderTs[0] = *currentLightSliders[0];
      sliderTs[1] = *currentLightSliders[1];
      sliderTs[2] = *currentLightSliders[2];
      sliderTs[3] = *currentLightSliders[3];
      sliderTs[4] = *currentLightSliders[4];
    
      glBindBuffer(GL_ARRAY_BUFFER, sliderGUI.vboTs);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * 5,
                   sliderGUI.ts,
                   GL_DYNAMIC_DRAW);

      for(int sliderIndex = 0;
        sliderIndex < 5;
        ++sliderIndex)
      {
          sliderIsSelecteds[sliderIndex] = 0;  
      }
      sliderIsSelecteds[currentSliderIndex] = 1;

      glBindBuffer(GL_ARRAY_BUFFER, sliderGUI.vboIsSelecteds);
      glBufferData(GL_ARRAY_BUFFER,
                 sizeof(int) * 5,
                 sliderGUI.isSelecteds,
                 GL_DYNAMIC_DRAW);

      glBindVertexArray(sliderGUI.VAO);
      glDrawArrays(GL_POINTS, 0, 5);
      glBindVertexArray(0);
    }

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
    Sleep(1);
  }

  par_shapes_free_mesh(shape.mesh);
  destroyRenderObject(&shape);

  par_shapes_free_mesh(shape2.mesh);
  destroyRenderObject(&shape2);

  glfwTerminate();

	return 0;
}
