struct shader
{
  GLuint VertexID;
  GLuint GeometryID;
  GLuint FragmentID;
  GLuint ID;

  char* VertexSource;
  char* GeometrySource;
  char* FragmentSource;

  char* Name;
};

void compileShader(shader* Shader)
{
  Shader->VertexID = glCreateShader(GL_VERTEX_SHADER);
  Shader->FragmentID = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(Shader->VertexID, 1,
                 &Shader->VertexSource, NULL);
  glShaderSource(Shader->FragmentID, 1,
                 &Shader->FragmentSource, NULL);

  glCompileShader(Shader->VertexID);
  glCompileShader(Shader->FragmentID);

  GLint Success;
  GLchar InfoLog[512];
  {
    glGetShaderiv(Shader->FragmentID, GL_COMPILE_STATUS, &Success);
    if(!Success)
    {
      glGetShaderInfoLog(Shader->FragmentID, 512, NULL, InfoLog);
      printf("Fragment Shader Compilation Failure: %s\n"
             "--: %s", Shader->Name, InfoLog);
    }

    glGetShaderiv(Shader->VertexID, GL_COMPILE_STATUS, &Success);
    if(!Success)
    {
      glGetShaderInfoLog(Shader->VertexID, 512, NULL, InfoLog);
      printf("Vertex Shader Compilation Failure: %s\n"
             "--: %s", Shader->Name, InfoLog);
    }
  }

  if(Shader->GeometrySource != NULL)
  {
    Shader->GeometryID = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(Shader->GeometryID, 1,
                   &Shader->GeometrySource, NULL);
    glCompileShader(Shader->GeometryID);

    {
      glGetShaderiv(Shader->GeometryID, GL_COMPILE_STATUS, &Success);
      if(!Success)
      {
        glGetShaderInfoLog(Shader->GeometryID, 512, NULL, InfoLog);
        printf("Geometry Shader Compilation Failure: %s\n"
               "--: %s", Shader->Name, InfoLog);
      }
    }
  }
}

void linkShader(shader* Shader)
{
  Shader->ID = glCreateProgram();
  glAttachShader(Shader->ID,
                 Shader->VertexID);
  glAttachShader(Shader->ID,
                 Shader->FragmentID);
  if(Shader->GeometrySource != NULL)
  {
    glAttachShader(Shader->ID,
                   Shader->GeometryID);
  }
  glLinkProgram(Shader->ID);

  {
    GLint Success;
    GLchar InfoLog[512];
    glGetProgramiv(Shader->ID, GL_LINK_STATUS, &Success);
    if(!Success)
    {
      glGetProgramInfoLog(Shader->ID, 512, NULL, InfoLog);
      printf("Shader Program Link Failure: %s", InfoLog);
    }
  }
}
