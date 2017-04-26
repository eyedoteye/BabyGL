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
#include "ShaderDefinitions.h"
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
  bool DebugMode = false;

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* Window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Duckbut",
                                        NULL, NULL);
  if(!Window)
  {
    printf("GLFW window creation failure.\n");
    glfwTerminate();
    return -1;
  }

  glfwMakeContextCurrent(Window);

  glewExperimental = GL_TRUE;
  if (glewInit() != GLEW_OK)
  {
    printf("GLEW initialization failure.\n");
    glfwTerminate();
    return -1;
  }

  glfwSetKeyCallback(Window, keyCallback);
  glfwSetCursorPosCallback(Window, mouseCallback);
  glfwSetInputMode(Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  int Width, Height;
  glfwGetFramebufferSize(Window, &Width, &Height);
  glViewport(0, 0, Width, Height);
  glEnable(GL_DEPTH_TEST);

  shader GPassShader;
  gpass_info GPassShaderInfo;  
  InitGPassShader(&GPassShader, &GPassShaderInfo, Width, Height); 

  shader LPassShader;
  lpass_info LPassShaderInfo;
  InitLPassShader(&LPassShader, &LPassShaderInfo);

  shader BloomShader;
  bloom_info BloomShaderInfo;
  InitBloomShader(&BloomShader, &BloomShaderInfo, Width, Height); 

  shader OutlineShader;
  outline_info OutlineShaderInfo;
  InitOutlineShader(&OutlineShader, &OutlineShaderInfo, Width, Height);

  shader GUIShader;
  InitGUIShader(&GUIShader);
  
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
  initCamera(&camera, (GLfloat)Width, (GLfloat)Height);

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

#define OBJECT_TYPE_NONE 0
#define OBJECT_TYPE_DEFAULT 1
#define OBJECT_TYPE_POINTLIGHT 2
  RenderObject LightShape;
  LightShape.objectType = OBJECT_TYPE_POINTLIGHT;
  LightShape.mesh = par_shapes_create_parametric_sphere(32, 32);
  LightShape.model = glm::scale(LightShape.model, glm::vec3(0.5f, 0.5f, 0.5f));
  initRenderObject(&LightShape);  

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
  while(!glfwWindowShouldClose(Window))
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
      DebugMode = !DebugMode;

    if(DebugMode)
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
    shape.model = glm::rotate(shape.model, ROTATION_SPEED * dT, glm::vec3(0.5f,
       .5f, 0.5f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT,
     glm::vec3(0.3f, 0.f, 0.6f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, 5.f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(1.f,
       0.f, 0.f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, -5.f));

    glBindFramebuffer(GL_FRAMEBUFFER, GPassShaderInfo.FrameBufferID);
    {
      glClearColor(0.f, 0.f, 0.f, 0.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
      glUseProgram(GPassShader.ID);
      {
        glUniformMatrix4fv(glGetUniformLocation(GPassShader.ID, "projection"),
          1, GL_FALSE, glm::value_ptr(camera.projection));
        glUniformMatrix4fv(glGetUniformLocation(GPassShader.ID,"view"), 1,
          GL_FALSE, glm::value_ptr(camera.view));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, perlinNoiseTextureID);
          drawRenderObject(&shape2, GPassShader.ID);
          drawRenderObject(&shape, GPassShader.ID);

        if(DebugMode)
        {
          for(int pointLightIndex = 0;
              pointLightIndex < POINT_LIGHT_COUNT;
              ++pointLightIndex)
          {
            drawPointLightDebugModel(LightShape,
             &pointLights[pointLightIndex], GPassShader.ID);
          }
        }
      } glUseProgram(0);
    }glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glClearColor(.4f, .6f, .2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if(DebugMode)
    {
      glUseProgram(BloomShader.ID);
      { 
        //Note: This bloom method is taken from learnopengl.com 
        GLboolean Vertical = false;
#define BLUR_ITERATIONS 10
        for(int Iteration = 0; Iteration < BLUR_ITERATIONS; ++Iteration)
        {
          glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1,
            &BloomShaderInfo.SubroutineIndices[Vertical]);

          glBindFramebuffer(GL_FRAMEBUFFER,
            BloomShaderInfo.FrameBufferIDs[Vertical]);
          {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,
              Iteration == 0 ?
              GPassShaderInfo.PointLightColorBufferID :
              BloomShaderInfo.ColorBufferIDs[!Vertical]);
            
            glBindVertexArray(quadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
             
          } glBindFramebuffer(GL_FRAMEBUFFER, 0);
          Vertical = !Vertical;
        }
      } glUseProgram(0);
    }

    glUseProgram(LPassShader.ID);
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, GPassShaderInfo.PositionBufferID);
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, GPassShaderInfo.NormalBufferID);
      glActiveTexture(GL_TEXTURE2);
      glBindTexture(GL_TEXTURE_2D, GPassShaderInfo.ColorBufferID);

      if(DebugMode)
      {
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D,
          BloomShaderInfo.ColorBufferIDs[(BLUR_ITERATIONS + 1) % 2]);

        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1,
          &LPassShaderInfo.DebugOnIndex);
      }
      else
      {
        glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1,
          &LPassShaderInfo.DebugOffIndex);
      }

      glUniform3fv(glGetUniformLocation(LPassShader.ID, "viewPosition"), 1,
        glm::value_ptr(camera.position));

      glUniform3fv(glGetUniformLocation(LPassShader.ID,
        "directionalLights[0].direction"), 1,
        glm::value_ptr(directionalLights[0].direction));
      glUniform3fv(glGetUniformLocation(LPassShader.ID,
        "directionalLights[0].color"), 1,
        glm::value_ptr(directionalLights[0].color));

      glUniform1f(glGetUniformLocation(LPassShader.ID,
        "directionalLights[0].intensityAmbient"),
        directionalLights[0].intensity.ambient / 255.f);
      glUniform1f(glGetUniformLocation(LPassShader.ID,
        "directionalLights[0].intensityDiffuse"),
        directionalLights[0].intensity.diffuse / 255.f);
      glUniform1f(glGetUniformLocation(LPassShader.ID,
        "directionalLights[0].intensitySpecular"),
        directionalLights[0].intensity.specular / 255.f);
    
      glBindBuffer(GL_UNIFORM_BUFFER, LPassShaderInfo.UniformBufferID);
      float buffer[32];
      int offset = 0;
      for(int pointLightIndex = 0;
          pointLightIndex < POINT_LIGHT_COUNT;
          ++pointLightIndex)
      {
        int size = sizeof(float) * 4; //vec3 color (vec4 min)
       memcpy(buffer + offset,
           glm::value_ptr(pointLights[pointLightIndex].color),
           size);
        offset += 4;

        //vec3 position (vec4 min)
        memcpy(buffer + offset,
          glm::value_ptr(pointLights[pointLightIndex].position),
          size);
        offset += 4;

        size = sizeof(float) * 2; //float attenuations[2]
        memcpy(buffer + offset,
          pointLights[pointLightIndex].attenuations,
          size);
        offset += 2;

        size = sizeof(float) * 3; //float intensities[3]
        memcpy(buffer + offset,
          pointLights[pointLightIndex].intensities,
          size);
        offset += 3;
        offset += 3;
      }
      glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(buffer), buffer);
      glBindBuffer(GL_UNIFORM_BUFFER, 0);

      glBindVertexArray(quadVAO);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    } glUseProgram(0);

   if(DebugMode)
   {
     glEnable(GL_STENCIL_TEST);

     glBindFramebuffer(GL_FRAMEBUFFER, OutlineShaderInfo.FrameBufferID);
     {
       glStencilFunc(GL_ALWAYS, 1, 0xFF);
       glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
       glStencilMask(0xFF);
       glDepthMask(GL_FALSE);
       glClearColor(1.f, 1.f, 1.f, 1.f);
       glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
       
       glUseProgram(OutlineShader.ID);
       {
         glUniformMatrix4fv(
           glGetUniformLocation(OutlineShader.ID, "projection"),
           1, GL_FALSE, glm::value_ptr(camera.projection));
         glUniformMatrix4fv(
           glGetUniformLocation(OutlineShader.ID, "view"),
           1, GL_FALSE, glm::value_ptr(camera.view));

         drawPointLightDebugModel(LightShape,
          &pointLights[currentPointLightIndex], 0.8f, OutlineShader.ID);

         glBindFramebuffer(GL_READ_FRAMEBUFFER,
           OutlineShaderInfo.FrameBufferID);
         glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
         {
           glBlitFramebuffer(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                             0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                             GL_STENCIL_BUFFER_BIT, GL_NEAREST);
           glBindFramebuffer(GL_FRAMEBUFFER, 0);

           glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
           glStencilMask(0x00);
           glDepthMask(GL_TRUE);
           glClear(GL_DEPTH_BUFFER_BIT);

           drawPointLightDebugModel(LightShape,
             &pointLights[currentPointLightIndex],
             OutlineShader.ID);

           glDisable(GL_STENCIL_TEST);
         } //glBindFramebuffer
       } glUseProgram(0);
     } glBindFramebuffer(GL_FRAMEBUFFER, 0);

     glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

     glUseProgram(GUIShader.ID); 
     {
       glUniform1f(glGetUniformLocation(GUIShader.ID, "onePixel"),
         1.f / SCREEN_WIDTH * 2);

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

       for(int SliderIndex = 0;
         SliderIndex < 5;
         ++SliderIndex)
       {
           sliderIsSelecteds[SliderIndex] = 0;  
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
      glfwSetWindowShouldClose(Window, GL_TRUE);
    }

    glfwSwapBuffers(Window);
    Sleep(1);
  }

  par_shapes_free_mesh(shape.mesh);
  destroyRenderObject(&shape);

  par_shapes_free_mesh(shape2.mesh);
  destroyRenderObject(&shape2);

  glfwTerminate();

	return 0;
}
