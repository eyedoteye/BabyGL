struct ShaderObject
{
  GLuint vertexShaderID;
  GLuint fragmentShaderID;
  GLuint shaderProgramID;

  char* vertexShaderSource;
  char* fragmentShaderSource;
};

void compileShaderObject(ShaderObject* shaderObject)
{
  shaderObject->vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  shaderObject->fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

  glShaderSource(shaderObject->vertexShaderID, 1,
                 &shaderObject->vertexShaderSource, NULL);
  glShaderSource(shaderObject->fragmentShaderID, 1,
                 &shaderObject->fragmentShaderSource, NULL);

  glCompileShader(shaderObject->vertexShaderID);
  glCompileShader(shaderObject->fragmentShaderID);

  {
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shaderObject->fragmentShaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(shaderObject->fragmentShaderID, 512, NULL, infoLog);
      printf("Fragment Shader Compilation Failure: %s", infoLog);
    }

    glGetShaderiv(shaderObject->vertexShaderID, GL_COMPILE_STATUS, &success);
    if(!success)
    {
      glGetShaderInfoLog(shaderObject->vertexShaderID, 512, NULL, infoLog);
      printf("Vertex Shader Compilation Failure: %s", infoLog);
    }
  }
}

void linkShaderObject(ShaderObject* shaderObject)
{
  shaderObject->shaderProgramID = glCreateProgram();
  glAttachShader(shaderObject->shaderProgramID,
                 shaderObject->vertexShaderID);
  glAttachShader(shaderObject->shaderProgramID,
                 shaderObject->fragmentShaderID);
  glLinkProgram(shaderObject->shaderProgramID);

  {
    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(shaderObject->shaderProgramID, GL_LINK_STATUS, &success);
    if(!success)
    {
      glGetProgramInfoLog(shaderObject->shaderProgramID, 512, NULL, infoLog);
      printf("Shader Program Link Failure: %s", infoLog);
    }
  }
}
