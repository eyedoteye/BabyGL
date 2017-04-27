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

#include "render_object.cpp"
#include "shader_object.cpp"
#include "shader_definitions.h"
#include "camera.cpp"
#include "perlin_texture.cpp"

#include <stdlib.h>
#include <time.h>

struct directional_light
{
  glm::vec3 Direction;
  glm::vec3 Color;
  struct
  {
    float Ambient;
    float Diffuse;
    float Specular;
  } Intensity;
};

struct point_light
{
  glm::vec3 Position;
  glm::vec3 Color;
  union
  {
    struct
    {
      float Ambient;
      float Diffuse;
      float Specular;
    } Intensity;
    float Intensities[3];
  };
  union
  {
    struct
    {
      float Linear;
      float Quadratic;
    } Attenuation;
    float Attenuations[2];
  };
};

void
FlattenPointLightSliders(
  point_light* PointLight,
  float** LightSliders)
{
  for(int IntensityIndex = 0;
      IntensityIndex < 3;
      ++IntensityIndex)
  {
    LightSliders[IntensityIndex] =
      &PointLight->Intensities[IntensityIndex];
  }
  for(int AttenuationIndex = 0;
      AttenuationIndex < 2;
      ++AttenuationIndex)
  {
    LightSliders[3 + AttenuationIndex] =
      &PointLight->Attenuations[AttenuationIndex];
  }
}

void DrawPointLightModel(
  render_object Renderable,
  point_light* PointLight,
  float Scale,
  GLuint ShaderProgramID)
{
  Renderable.ModelMatrix =
    glm::translate(Renderable.ModelMatrix, PointLight->Position);
  Renderable.ModelMatrix =
    glm::scale(Renderable.ModelMatrix, glm::vec3(Scale, Scale, Scale));
  Renderable.Color = PointLight->Color;
  DrawRenderObject(&Renderable, ShaderProgramID);
}

void DrawPointLightModel(
  render_object Renderable,
  point_light* PointLight,
  GLuint ShaderProgramID)
{
  Renderable.ModelMatrix =
    glm::translate(Renderable.ModelMatrix, PointLight->Position);
  Renderable.Color = PointLight->Color;
  DrawRenderObject(&Renderable, ShaderProgramID);
}

struct slider_gui
{
  GLuint VAO;
  GLuint PositionVBO, ValueVBO, SelectionVBO;

  int Count;
  float* Positions;
  float* Values;
  int* Selections; // Note: No Booleans in GLSL
};

void
InitSliderGUI(slider_gui *SliderGUI, int Count)
{
  glGenVertexArrays(1, &SliderGUI->VAO);
  glGenBuffers(1, &SliderGUI->PositionVBO);
  glGenBuffers(1, &SliderGUI->ValueVBO);
  glGenBuffers(1, &SliderGUI->SelectionVBO);

  glBindVertexArray(SliderGUI->VAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, SliderGUI->PositionVBO);
    {
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * Count * 5,
                   SliderGUI->Positions,
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
    }
    glBindBuffer(GL_ARRAY_BUFFER, SliderGUI->ValueVBO);
    {
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(float) * Count,
                   SliderGUI->Values,
                   GL_DYNAMIC_DRAW);
      glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE,
                            sizeof(float),
                            (GLvoid*)0);
      glEnableVertexAttribArray(3);

      glBindBuffer(GL_ARRAY_BUFFER, SliderGUI->SelectionVBO);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(int) * Count,
                   SliderGUI->Selections,
                   GL_DYNAMIC_DRAW);
      glVertexAttribPointer(4, 1, GL_INT, GL_FALSE,
                            sizeof(int),
                          (GLvoid*)0);
      glEnableVertexAttribArray(4);
    } glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

#pragma warning(push)
#pragma warning(disable:4100)
static bool KeysPressed[1024];
static bool KeysHeld[1024];
void
KeyCallback(
  GLFWwindow* window,
  int Key,
  int ScanCode,
  int Action,
  int Mode)
{
  if(Key == GLFW_KEY_ESCAPE)
  {
    glfwSetWindowShouldClose(window, GL_TRUE);
  }

  if(Action == GLFW_PRESS)
  {
    KeysPressed[Key] = true;
    KeysHeld[Key] = true; 
  }
  else if(Action == GLFW_RELEASE)
  {
    KeysHeld[Key] = false;
  }
}

static GLfloat MouseCoords[2];
void
MouseCallback(GLFWwindow* Window, double XPos, double YPos)
{
  MouseCoords[0] = (GLfloat)XPos;
  MouseCoords[1] = (GLfloat)YPos;
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

  glfwSetKeyCallback(Window, KeyCallback);
  glfwSetCursorPosCallback(Window, MouseCallback);
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
  
  GLuint QuadVAO;
  GLuint QuadVBO;
  GLfloat QuadVertices[] = {
    -1.f, 1.f, 0.f,     0.f, 1.f,
    -1.f, -1.f, 0.f,    0.f, 0.f,
    1.f, 1.f, 0.f,      1.f, 1.f,
    1.f, -1.f, 0.f,      1.f, 0.f
  };

  glGenVertexArrays(1, &QuadVAO);
  glGenBuffers(1, &QuadVBO);
  glBindVertexArray(QuadVAO);
  glBindBuffer(GL_ARRAY_BUFFER, QuadVBO);
  {
    glBufferData(GL_ARRAY_BUFFER, sizeof(QuadVertices), &QuadVertices,
      GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
      (GLvoid*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat),
      (GLvoid*)(3 * sizeof(GLfloat)));
  } glBindBuffer(GL_ARRAY_BUFFER, 0);

#define P2F(X, DIM) ((X)/(float)(DIM)) * 2.f - 1.f
  float SliderGeometry[] = {
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

  float SliderValues[5] = {};
  int SliderSelections[5] = {};

  slider_gui SliderGUI = {};
  SliderGUI.Positions = SliderGeometry;
  SliderGUI.Values = SliderValues;
  SliderGUI.Selections = SliderSelections;
  InitSliderGUI(&SliderGUI, 5);

  GLuint NoiseTextureID;
  glGenTextures(1, &NoiseTextureID);
  Generate2DPerlinNoise(NoiseTextureID, 420);

  camera Camera;
  InitCamera(&Camera, (GLfloat)Width, (GLfloat)Height);

  directional_light DirectionalLights[1] = {};
  DirectionalLights[0].Direction = glm::vec3(0.f, 5.f, -2.f);
  DirectionalLights[0].Color = glm::vec3(0.f, 1.f, 0.f);
  DirectionalLights[0].Intensity.Ambient = 255 * 0.1f;
  DirectionalLights[0].Intensity.Diffuse = 255 * 0.5f;
  DirectionalLights[0].Intensity.Specular = 255 * 0.5f;

#define POINT_LIGHT_COUNT 2
  point_light PointLights[POINT_LIGHT_COUNT] = {};
  PointLights[0].Position = glm::vec3(0.f, 5.f, -2.f);
  PointLights[0].Color = glm::vec3(1.f, 0.1f, 0.1f);
  PointLights[1].Position = glm::vec3(0.f, 1.f, -8.f);
  PointLights[1].Color = glm::vec3(1.f, 1.f, 1.f);

#define OBJECT_TYPE_NONE 0
#define OBJECT_TYPE_DEFAULT 1
#define OBJECT_TYPE_POINTLIGHT 2
  render_object LightShape;
  LightShape.Type = OBJECT_TYPE_POINTLIGHT;
  LightShape.Mesh = par_shapes_create_parametric_sphere(32, 32);
  LightShape.ModelMatrix =
    glm::scale(LightShape.ModelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
  InitRenderObject(&LightShape);  

  render_object WhiteSphere;
  WhiteSphere.Type = OBJECT_TYPE_DEFAULT;
  WhiteSphere.Mesh = par_shapes_create_parametric_sphere(32, 32);
  WhiteSphere.ModelMatrix =
    glm::translate(WhiteSphere.ModelMatrix, glm::vec3(0.f, 0.f, 0.f));
  WhiteSphere.Color = glm::vec3(0.7f); 
  InitRenderObject(&WhiteSphere);

  render_object TealSphere;
  TealSphere.Type = OBJECT_TYPE_DEFAULT;
  TealSphere.Mesh = par_shapes_create_parametric_sphere(32, 32);
  TealSphere.ModelMatrix =
    glm::translate(TealSphere.ModelMatrix, glm::vec3(0.f, 1.f, -3.f));
  TealSphere.Color = glm::vec3(.2f, 1.f, 1.f);
  InitRenderObject(&TealSphere);

  char CurrentPointLightIndex = 0;
  char CurrentSliderIndex = 0;
  float* CurrentLightSliders[5] = {};
  FlattenPointLightSliders(&PointLights[CurrentPointLightIndex],
                           CurrentLightSliders);
    
  GLfloat MouseCoordsAtLastFrameStart[2];
  MouseCoordsAtLastFrameStart[0] = MouseCoords[0];
  MouseCoordsAtLastFrameStart[1] = MouseCoords[1];

  GLfloat DT = 0.f;
  GLfloat TimeAtLastFrameStart = 0.f;
  while(!glfwWindowShouldClose(Window))
  {
    GLfloat TimeNow = (GLfloat)glfwGetTime();
    DT = TimeNow - TimeAtLastFrameStart;

    TimeAtLastFrameStart = TimeNow;

    memset(KeysPressed, 0, 1024 * sizeof(bool)); 

    glfwPollEvents();

    GLfloat MouseCoordsNow[2];
    MouseCoordsNow[0] = MouseCoords[0];
    MouseCoordsNow[1] = MouseCoords[1];

    UpdateCamera(&Camera,
                 MouseCoordsNow[0] - MouseCoordsAtLastFrameStart[0],
                 MouseCoordsNow[1] - MouseCoordsAtLastFrameStart[1],
                 KeysHeld,
                 DT);

    MouseCoordsAtLastFrameStart[0] = MouseCoordsNow[0];
    MouseCoordsAtLastFrameStart[1] = MouseCoordsNow[1];

#define SLIDERSPEED (25 / 255.f)
    if(KeysPressed[GLFW_KEY_TAB])
    {
      if(KeysPressed[GLFW_MOD_SHIFT])
        --CurrentPointLightIndex;     
      else
        ++CurrentPointLightIndex;
      
      CurrentPointLightIndex = (CurrentPointLightIndex + POINT_LIGHT_COUNT)
                                  % POINT_LIGHT_COUNT;
      FlattenPointLightSliders(&PointLights[CurrentPointLightIndex],
                               CurrentLightSliders);
    }

    if(KeysPressed[GLFW_KEY_GRAVE_ACCENT])
      DebugMode = !DebugMode;

    if(DebugMode)
    {
      if(KeysPressed[GLFW_KEY_1])
        CurrentSliderIndex = 0;
      else if(KeysPressed[GLFW_KEY_2])
        CurrentSliderIndex = 1;
      else if(KeysPressed[GLFW_KEY_3])
        CurrentSliderIndex = 2;
      else if(KeysPressed[GLFW_KEY_4])
        CurrentSliderIndex = 3;
      else if(KeysPressed[GLFW_KEY_5])
        CurrentSliderIndex = 4;

      float* CurrentLightSlider = CurrentLightSliders[CurrentSliderIndex];
      if(KeysHeld[GLFW_KEY_Q])
        *CurrentLightSlider -= SLIDERSPEED * DT;
      if(KeysHeld[GLFW_KEY_E])
        *CurrentLightSlider += SLIDERSPEED * DT;

      if(*CurrentLightSlider < 0)
        *CurrentLightSlider = 0;
      if(*CurrentLightSlider > 1.f)
        *CurrentLightSlider = 1;
    }

#define ROTATION_SPEED .2f
    WhiteSphere.ModelMatrix =
      glm::rotate(WhiteSphere.ModelMatrix,
                  ROTATION_SPEED * DT,
                  glm::vec3(0.5f, .5f, 0.5f));
    TealSphere.ModelMatrix =
      glm::rotate(TealSphere.ModelMatrix,
                  ROTATION_SPEED * DT,
                  glm::vec3(0.3f, 0.f, 0.6f));
    TealSphere.ModelMatrix =
      glm::translate(TealSphere.ModelMatrix,
                     glm::vec3(0.f, 0.f, 5.f));
    TealSphere.ModelMatrix =
      glm::rotate(TealSphere.ModelMatrix,
                  ROTATION_SPEED * DT,
                  glm::vec3(1.f, 0.f, 0.f));
    TealSphere.ModelMatrix =
      glm::translate(TealSphere.ModelMatrix,
                     glm::vec3(0.f, 0.f, -5.f));

    glBindFramebuffer(GL_FRAMEBUFFER, GPassShaderInfo.FrameBufferID);
    {
      glClearColor(0.f, 0.f, 0.f, 0.f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
      glUseProgram(GPassShader.ID);
      {
        glUniformMatrix4fv(
          glGetUniformLocation(GPassShader.ID, "ProjectionMatrix"),
          1, GL_FALSE,
          glm::value_ptr(Camera.Projection));
        glUniformMatrix4fv(
          glGetUniformLocation(GPassShader.ID,"ViewMatrix"),
          1, GL_FALSE,
          glm::value_ptr(Camera.View));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, NoiseTextureID);
        {
          DrawRenderObject(&TealSphere, GPassShader.ID);
          DrawRenderObject(&WhiteSphere, GPassShader.ID);
        } // TextureBindEnd

        if(DebugMode)
        {
          for(int PointLightIndex = 0;
              PointLightIndex < POINT_LIGHT_COUNT;
              ++PointLightIndex)
          {
            DrawPointLightModel(
              LightShape,
              &PointLights[PointLightIndex],
              GPassShader.ID);
          }
        }
      } glUseProgram(0);
    }glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if(DebugMode)
    {
      glUseProgram(BloomShader.ID);
      { 
        //Note: This bloom method is taken from learnopengl.com 
#define BLUR_ITERATIONS 10
        GLboolean Vertical = false;
        for(int Iteration = 0;
            Iteration < BLUR_ITERATIONS;
            ++Iteration)
        {
          glUniformSubroutinesuiv(
            GL_FRAGMENT_SHADER, 1,
            &BloomShaderInfo.SubroutineIndices[Vertical]);

          glBindFramebuffer(
            GL_FRAMEBUFFER,
            BloomShaderInfo.FrameBufferIDs[Vertical]);
          {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(
              GL_TEXTURE_2D,
              Iteration == 0 ?
                GPassShaderInfo.PointLightColorTextureID :
                BloomShaderInfo.ColorTextureIDs[!Vertical]);
            
            glBindVertexArray(QuadVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
             
          } glBindFramebuffer(GL_FRAMEBUFFER, 0);
          Vertical = !Vertical;
        }
      } glUseProgram(0);
    }

   glClearColor(.4f, .6f, .2f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glUseProgram(LPassShader.ID);
   {
     if(DebugMode)
     {
       glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1,
                               &LPassShaderInfo.DebugOnIndex);
     }
     else
     {
       glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 1,
                               &LPassShaderInfo.DebugOffIndex);
     }

     glUniform3fv(
       glGetUniformLocation(LPassShader.ID, "ViewPosition"),
       1, glm::value_ptr(Camera.Position));

     glUniform3fv(
       glGetUniformLocation(LPassShader.ID, "DirectionalLights[0].Direction"),
       1, glm::value_ptr(DirectionalLights[0].Direction));
     glUniform3fv(
       glGetUniformLocation(LPassShader.ID, "DirectionalLights[0].Color"),
       1, glm::value_ptr(DirectionalLights[0].Color)); 

     glUniform1f(
       glGetUniformLocation(LPassShader.ID,
                            "DirectionalLights[0].IntensityAmbient"),
       DirectionalLights[0].Intensity.Ambient / 255.f);
     glUniform1f(
       glGetUniformLocation(LPassShader.ID,
                            "DirectionalLights[0].IntensityDiffuse"),
       DirectionalLights[0].Intensity.Diffuse / 255.f);
     glUniform1f(
      glGetUniformLocation(LPassShader.ID,
                           "DirectionalLights[0].IntensitySpecular"),
      DirectionalLights[0].Intensity.Specular / 255.f);

     glBindBuffer(GL_UNIFORM_BUFFER, LPassShaderInfo.UniformBufferID);
     {
       float Buffer[32];
       int Offset = 0;
       for(int PointLightIndex = 0;
           PointLightIndex < POINT_LIGHT_COUNT;
           ++PointLightIndex)
       {
         int Size = sizeof(float) * 4; //vec3 Color (vec4 min)
         memcpy(Buffer + Offset,
                glm::value_ptr(PointLights[PointLightIndex].Color), Size);
         Offset += 4;

         //vec3 Position (vec4 min)
         memcpy(Buffer + Offset,
                glm::value_ptr(PointLights[PointLightIndex].Position), Size);
         Offset += 4;

         Size = sizeof(float) * 2; //float Attenuations[2]
         memcpy(Buffer + Offset,
                PointLights[PointLightIndex].Attenuations, Size);
         Offset += 2;

         Size = sizeof(float) * 3; //float Intensities[3]
         memcpy(Buffer + Offset,
                PointLights[PointLightIndex].Intensities, Size);
         Offset += 3;

         // Note: Additional offset for stride alignment
         Offset += 3;
       }
       glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Buffer), Buffer);
     }
     glBindBuffer(GL_UNIFORM_BUFFER, 0);

     glActiveTexture(GL_TEXTURE0);
     glBindTexture(GL_TEXTURE_2D, GPassShaderInfo.PositionTextureID);
     glActiveTexture(GL_TEXTURE1);
     glBindTexture(GL_TEXTURE_2D, GPassShaderInfo.NormalTextureID);
     glActiveTexture(GL_TEXTURE2);
     glBindTexture(GL_TEXTURE_2D, GPassShaderInfo.ColorTextureID);
     glActiveTexture(GL_TEXTURE3);
     glBindTexture(
       GL_TEXTURE_2D,
       BloomShaderInfo.ColorTextureIDs[(BLUR_ITERATIONS + 1) % 2]);
     {
       glBindVertexArray(QuadVAO);
       glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
     } // TextureBindEnd
   } glUseProgram(0);

   if(DebugMode)
   {
     glEnable(GL_STENCIL_TEST);

     glUseProgram(OutlineShader.ID);
     {
       glUniformMatrix4fv(
         glGetUniformLocation(OutlineShader.ID, "ProjectionMatrix"),
         1, GL_FALSE, glm::value_ptr(Camera.Projection));
       glUniformMatrix4fv(
         glGetUniformLocation(OutlineShader.ID, "ViewMatrix"),
         1, GL_FALSE, glm::value_ptr(Camera.View));

       glBindFramebuffer(GL_FRAMEBUFFER, OutlineShaderInfo.FrameBufferID);
       {
         glStencilFunc(GL_ALWAYS, 1, 0xFF);
         glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
         glStencilMask(0xFF);
         glDepthMask(GL_FALSE);
         glClearColor(1.f, 1.f, 1.f, 1.f);
         glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
       
         DrawPointLightModel(
           LightShape,
           &PointLights[CurrentPointLightIndex], 0.8f, OutlineShader.ID);
       }
       glBindFramebuffer(GL_READ_FRAMEBUFFER, OutlineShaderInfo.FrameBufferID);
       glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
       { 
         glBlitFramebuffer(
           0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
           0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
           GL_STENCIL_BUFFER_BIT, GL_NEAREST);
       } glBindFramebuffer(GL_FRAMEBUFFER, 0);

       glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
       glStencilMask(0x00);
       glDepthMask(GL_TRUE);
       glClear(GL_DEPTH_BUFFER_BIT);

       DrawPointLightModel(LightShape,
         &PointLights[CurrentPointLightIndex],
         OutlineShader.ID);

       glDisable(GL_STENCIL_TEST);
     } glUseProgram(0);

     glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

     glUseProgram(GUIShader.ID); 
     {
       glUniform1f(
         glGetUniformLocation(GUIShader.ID, "OnePixel"),
         1.f / SCREEN_WIDTH * 2);

       SliderValues[0] = *CurrentLightSliders[0];
       SliderValues[1] = *CurrentLightSliders[1];
       SliderValues[2] = *CurrentLightSliders[2];
       SliderValues[3] = *CurrentLightSliders[3];
       SliderValues[4] = *CurrentLightSliders[4];
     
       glBindBuffer(GL_ARRAY_BUFFER, SliderGUI.ValueVBO);
       {
         glBufferData(
           GL_ARRAY_BUFFER,
           sizeof(float) * 5,
           SliderGUI.Values,
           GL_DYNAMIC_DRAW);
       }

       for(int SliderIndex = 0;
           SliderIndex < 5;
           ++SliderIndex)
       {
         SliderSelections[SliderIndex] = 0;  
       }
       SliderSelections[CurrentSliderIndex] = 1;

       glBindBuffer(GL_ARRAY_BUFFER, SliderGUI.SelectionVBO);
       {
         glBufferData(
           GL_ARRAY_BUFFER,
           sizeof(int) * 5,
           SliderGUI.Selections,
           GL_DYNAMIC_DRAW);
       }

       glBindVertexArray(SliderGUI.VAO);
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

  par_shapes_free_mesh(WhiteSphere.Mesh);
  destroyRenderObject(&WhiteSphere);

  par_shapes_free_mesh(TealSphere.Mesh);
  destroyRenderObject(&TealSphere);

  glfwTerminate();

	return 0;
}
