#pragma once

void
SetupBasicTexture(
  GLint InternalFormat,
  GLenum Format,
  GLenum Attachment,
  GLuint TextureID,
  GLuint Width,
  GLuint Height)
{
  glTexImage2D(GL_TEXTURE_2D, 0, InternalFormat, Width, Height, 0,
    Format, GL_FLOAT, NULL);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glFramebufferTexture2D(
    GL_FRAMEBUFFER, Attachment, GL_TEXTURE_2D, TextureID, 0); 
}

struct gpass_info
{
  GLuint FrameBufferID; //Note: In OpenGL Frame Buffer is framebuffer

  union
  {
    GLuint DefaultBuffers[3];
    struct
    {
      GLuint PositionTextureID;
      GLuint NormalTextureID;
      GLuint ColorTextureID;
    };
  };
  //Todo: Move into seperate deferred set-up.
  GLuint PointLightColorTextureID;
};

void
CreateDeferredBuffer(
  gpass_info* DeferredBuffer,
  GLuint Width,
  GLuint Height)
{
  glGenFramebuffers(1, &DeferredBuffer->FrameBufferID);
  glBindFramebuffer(GL_FRAMEBUFFER, DeferredBuffer->FrameBufferID);
  {
    glGenTextures(3, DeferredBuffer->DefaultBuffers); 

    for(int BufferIndex = 0; BufferIndex < 3; ++BufferIndex)
    {
      glBindTexture(GL_TEXTURE_2D,
        DeferredBuffer->DefaultBuffers[BufferIndex]);
      {
        SetupBasicTexture(GL_RGB16F, GL_RGB, GL_COLOR_ATTACHMENT0 + BufferIndex,
          DeferredBuffer->DefaultBuffers[BufferIndex], Width, Height);
      } glBindTexture(GL_TEXTURE_2D, 0);
    }

    glGenTextures(1, &DeferredBuffer->PointLightColorTextureID);
    glBindTexture(GL_TEXTURE_2D, DeferredBuffer->PointLightColorTextureID);
    {
      SetupBasicTexture(GL_RGBA16F, GL_RGBA, GL_COLOR_ATTACHMENT3,
        DeferredBuffer->PointLightColorTextureID, Width, Height);
    } glBindTexture(GL_TEXTURE_2D, 0);

    GLuint Attachments[4] = {
      GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2,
      GL_COLOR_ATTACHMENT3 };
    glDrawBuffers(4, Attachments);

    GLuint RboDepth;
    glGenRenderbuffers(1, &RboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, RboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Width,
      Height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
      GL_RENDERBUFFER, RboDepth);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
      printf("Framebuffer is broken: GPass!\n");
  } glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
InitGPassShader(
  shader* Shader,
  gpass_info* Info,
  GLuint Width,
  GLuint Height)
{
  Shader->Name = "GPass";
  Shader->VertexSource =
    #include "g_pass.vs"
  Shader->GeometrySource = NULL;
  Shader->FragmentSource =
    #include "g_pass.fs"

  compileShader(Shader);
  linkShader(Shader);

  glUseProgram(Shader->ID);
  {
    glUniform1i(glGetUniformLocation(Shader->ID, "PerlinNoise"), 0);
  } glUseProgram(0);

  CreateDeferredBuffer(Info, Width, Height);
}

// Todo: Split PointLight handling into its own shader.
struct lpass_info
{
  GLuint DebugOnIndex;
  GLuint DebugOffIndex;

  GLuint UniformBufferID;
};

#define POINT_LIGHT_MAX 5
void
InitLPassShader(
  shader* Shader,
  lpass_info* Info)
{
  Shader->Name = "LPass";
  Shader->VertexSource =
    #include "l_pass.vs"
  Shader->GeometrySource = NULL;
  Shader->FragmentSource =
    #include "l_pass.fs"

  compileShader(Shader);
  linkShader(Shader);

  glUseProgram(Shader->ID);
  {
    glUniform1i(glGetUniformLocation(Shader->ID, "PositionTexture"), 0);
    glUniform1i(glGetUniformLocation(Shader->ID, "NormalTexture"), 1);
    glUniform1i(glGetUniformLocation(Shader->ID, "ColorTexture"), 2);
    // Todo: Split PointLight handling into its own shader.
    glUniform1i(glGetUniformLocation(Shader->ID, "PointLightColorTexture"), 3);
    
    Info->DebugOnIndex = glGetSubroutineIndex(Shader->ID,
      GL_FRAGMENT_SHADER, "DebugOn"); 
    Info->DebugOffIndex = glGetSubroutineIndex(Shader->ID,
      GL_FRAGMENT_SHADER, "DebugOff");
  } glUseProgram(0);  

  glUniformBlockBinding(Shader->ID,
    glGetUniformBlockIndex(Shader->ID, "PointLightUBO"), 0);
  
  glGenBuffers(1, &Info->UniformBufferID);

  int StructSize = // Alignment Offset Description
    sizeof(float) * 4     // 16        0      vec3 color (vec4 min)
  + sizeof(float) * 4     // 16        16     vec3 position (vec4 min)
  + sizeof(float)         // 4         32     float attenuationLinear
  + sizeof(float)         // 4         36     float attenuationQuadratic
  + sizeof(float)         // 4         40     float intensityAmbient
  + sizeof(float)         // 4         44     float intensityDiffuse
  + sizeof(float);        // 4         48     float intensitySpecular
  int Alignment = sizeof(float) * 4; // vec4 min for structs
  StructSize = (int)((StructSize + (Alignment - 1)) / Alignment) * Alignment;
  int TotalSize = StructSize * POINT_LIGHT_MAX;

  glBindBuffer(GL_UNIFORM_BUFFER, Info->UniformBufferID);
  glBufferData(GL_UNIFORM_BUFFER, TotalSize, NULL, GL_DYNAMIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, Info->UniformBufferID, 0, TotalSize);
}

struct bloom_info
{
  union
  {
    GLuint FrameBufferIDs[2];
    struct
    {
      GLuint FrameBufferHorizontalID;
      GLuint FrameBufferVerticalID;
    };
  };
  union
  {
    GLuint ColorTextureIDs[2];
    struct
    {
      GLuint ColorBufferHorizontalID;
      GLuint ColorBufferVerticalID;
    };
  };
  union
  {
    GLuint SubroutineIndices[2];
    struct
    {
      GLuint SubroutineHorizontalIndex;
      GLuint SubroutineVerticalIndex;
    };
  };
};

void
InitBloomShader(
  shader* Shader,
  bloom_info* Info,
  GLuint Width,
  GLuint Height)
{
  Shader->Name = "Bloom";
  Shader->VertexSource =
    #include "bloom.vs"
  Shader->GeometrySource = NULL;
  Shader->FragmentSource =
    #include "bloom.fs"

  compileShader(Shader);
  linkShader(Shader);

  glGenFramebuffers(2, Info->FrameBufferIDs);
  glGenTextures(2, Info->ColorTextureIDs);
  
  glUseProgram(Shader->ID);
  {
    for(int AxisIndex = 0;
        AxisIndex < 2;
        ++AxisIndex)
    {
      glBindFramebuffer(GL_FRAMEBUFFER, Info->FrameBufferIDs[AxisIndex]);
      {
        glBindTexture(GL_TEXTURE_2D, Info->ColorTextureIDs[AxisIndex]);
        {
          SetupBasicTexture(GL_RGBA16F, GL_RGBA, GL_COLOR_ATTACHMENT0,
            Info->ColorTextureIDs[AxisIndex], Width, Height);

          if(glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
            GL_FRAMEBUFFER_COMPLETE)
          {
            printf("Framebuffer is broken: Bloom:%i\n", AxisIndex);
          }
        } glBindTexture(GL_TEXTURE_2D, 0);
      } glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    Info->SubroutineHorizontalIndex = glGetSubroutineIndex(
      Shader->ID, GL_FRAGMENT_SHADER, "BlurHorizontal");
    Info->SubroutineVerticalIndex = glGetSubroutineIndex(
      Shader->ID, GL_FRAGMENT_SHADER, "BlurVertical");
  } glUseProgram(0);  
}

struct outline_info
{
  GLuint FrameBufferID;
  GLuint ColorTextureID;
  GLuint StencilTextureID;
};

void
InitOutlineShader(
  shader* Shader,
  outline_info* Info,
  GLuint Width,
  GLuint Height)
{
  Shader->Name = "Outline";
  Shader->VertexSource =
    #include "outline.vs"
  Shader->GeometrySource = NULL;
  Shader->FragmentSource =
    #include "outline.fs"

  compileShader(Shader);
  linkShader(Shader);

  glGenFramebuffers(1, &Info->FrameBufferID);
  glBindFramebuffer(GL_FRAMEBUFFER, Info->FrameBufferID);
  {
    glGenTextures(1, &Info->ColorTextureID);
    glBindTexture(GL_TEXTURE_2D, Info->ColorTextureID);
    {
      SetupBasicTexture(GL_RGB16F, GL_RGB, GL_COLOR_ATTACHMENT0,
        Info->ColorTextureID, Width, Height);
    } glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &Info->StencilTextureID);
    glBindRenderbuffer(GL_RENDERBUFFER, Info->StencilTextureID);
    {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_STENCIL_INDEX8, Width,
        Height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
        GL_RENDERBUFFER, Info->StencilTextureID);
    } glBindRenderbuffer(GL_RENDERBUFFER, 0);
  } glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
InitGUIShader(shader* Shader)
{
  Shader->Name = "GUI";
  Shader->VertexSource =
    #include "gui.vs"
  Shader->GeometrySource =
    #include "gui.gs"
  Shader->FragmentSource =
    #include "gui.fs"

  compileShader(Shader);
  linkShader(Shader);
}
