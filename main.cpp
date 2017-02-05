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

#include "renderObject.cpp"

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

struct Camera
{
  glm::mat4 projection;
  glm::mat4 view;

  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;

  GLfloat yaw;
  GLfloat pitch;
};

void initCamera(Camera* camera, GLfloat width, GLfloat height)
{
  camera->projection = glm::perspective(45.f, width / height, 0.1f, 100.f);

  camera->position = glm::vec3(0.f, 0.f, -5.f);
  camera->front = glm::vec3(0.f, 0.f, 1.f);
  camera->up = glm::vec3(0.f, 1.f, 0.f);

  camera->yaw = 90.f;
  camera->pitch = 0.f;
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

void updateCamera(Camera* camera, GLfloat xOffset, GLfloat yOffset, float dT)
{
  if(keys[GLFW_KEY_W])
    camera->position += camera->front * dT;
  if(keys[GLFW_KEY_S])
    camera->position -= camera->front * dT;
  if(keys[GLFW_KEY_A])
    camera->position -= glm::normalize(glm::cross(camera->front, camera->up)) * dT;
  if(keys[GLFW_KEY_D])
    camera->position += glm::normalize(glm::cross(camera->front, camera->up)) * dT;
  if(keys[GLFW_KEY_SPACE])
    camera->position += camera->up * dT;
  if(keys[GLFW_KEY_LEFT_SHIFT])
    camera->position -= camera->up * dT;

  camera->pitch -= yOffset * 0.05f;
  camera->yaw += xOffset * 0.05f;

  camera->pitch = camera->pitch > 89.f ? 89.f : camera->pitch;
  camera->pitch = camera->pitch < -89.f ? -89.f : camera->pitch;

  glm::vec3 front;
  front.x = cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
  front.y = sin(glm::radians(camera->pitch));
  front.z = sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch));
  camera->front = glm::normalize(front);

  camera->view = glm::lookAt(camera->position,
                             camera->position + camera->front,
                             camera->up);
}

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	GLFWwindow* window = glfwCreateWindow(640,480, "Duckbut", NULL, NULL);
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

  GLchar* vertexShaderSource =
    "#version 400 core\n"
    "out vec3 vertexNormal;"
    "out vec3 vertexPosition;"

    "layout (location = 0) in vec3 position;"
    "layout (location = 1) in vec3 normal;"

    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 projection;"

    "void main()"
    "{"
    " gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.f);"
    " vertexNormal = mat3(transpose(inverse(model))) * normal;"
    " vertexPosition = vec3(model * vec4(position, 1.f));"
    "}";

  GLchar* fragmentShaderSource =
    "#version 400 core\n"
    "out vec4 color;"
    "in vec3 vertexNormal;"
    "in vec3 vertexPosition;"

    "uniform vec3 viewPosition;"
    "uniform vec3 lightPosition;"
    "uniform vec3 lightColor;"
    "uniform vec3 objectColor;"
    "void main()"
    "{"
    " vec3 ambient = 0.1f * lightColor;"

    " vec3 normal = normalize(vertexNormal);"
    " vec3 lightDirection = normalize(lightPosition - vertexPosition);"
    " vec3 diffuse = max(dot(normal, lightDirection), 0.f) * lightColor;"

    " vec3 viewDirection = normalize(viewPosition - vertexPosition);"
    " vec3 reflectDirection = reflect(-lightDirection, normal);"
    " vec3 specular = 0.5f * pow(max(dot(viewDirection, reflectDirection), 0.f), 32) * lightColor;"

    "color = vec4((ambient + diffuse + specular) * objectColor, 1.f);"
    "}";

  ShaderObject phongShader;
  phongShader.vertexShaderSource = vertexShaderSource;
  phongShader.fragmentShaderSource = fragmentShaderSource;
  compileShaderObject(&phongShader);
  linkShaderObject(&phongShader);

  Camera camera;
  initCamera(&camera, (GLfloat)width, (GLfloat)height);

  glm::vec3 lightPosition = glm::vec3(0.f, 5.f, -2.f);
  glm::vec3 lightColor = glm::vec3(1.f);

  RenderObject shape;
  initRenderObject(&shape, par_shapes_create_cube());
  shape.model = glm::translate(shape.model, glm::vec3(0.f, 0.f, 0.f));

  RenderObject shape2;
  initRenderObject(&shape2, par_shapes_create_tetrahedron());
  shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 1.f, -3.f));
  shape2.color = glm::vec3(.2f, 1.f, 1.f);

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
                 dT);

    mouseCoordsAtLastFrameStart[0] = mouseCoordsNow[0];
    mouseCoordsAtLastFrameStart[1] = mouseCoordsNow[1];

#define ROTATION_SPEED 1.f
    shape.model = glm::rotate(shape.model, ROTATION_SPEED * dT, glm::vec3(0.5f, .5f, 0.5f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(0.3f, 0.f, 0.6f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, 5.f));
    shape2.model = glm::rotate(shape2.model, ROTATION_SPEED * dT, glm::vec3(1.f, 0.f, 0.f));
    shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 0.f, -5.f));

    glUseProgram(phongShader.shaderProgramID);
    glClearColor(0.4f, 0.6f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    GLint projectionLocation = glGetUniformLocation(phongShader.shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(camera.projection));
    GLint viewLocation = glGetUniformLocation(phongShader.shaderProgramID, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(camera.view));

    GLint viewPositionLocation = glGetUniformLocation(phongShader.shaderProgramID, "viewPosition");
    glUniform3fv(viewPositionLocation, 1, glm::value_ptr(camera.position));
    GLint lightPositionLocation = glGetUniformLocation(phongShader.shaderProgramID, "lightPosition");
    glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition));
    GLint lightColorLocation = glGetUniformLocation(phongShader.shaderProgramID, "lightColor");
    glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));

    drawRenderObject(&shape2, phongShader.shaderProgramID);
    drawRenderObject(&shape, phongShader.shaderProgramID);

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
