#include <stdio.h>

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <par/par_shapes.h>

struct RenderableMesh
{
  par_shapes_mesh* mesh;
  GLuint VAO, VBO, EBO;
};

void initRenderableMesh(RenderableMesh *mesh, par_shapes_mesh *shapeMesh)
{
  mesh->mesh = shapeMesh;

  glGenVertexArrays(1, &mesh->VAO);
  glGenBuffers(1, &mesh->VBO);
  glGenBuffers(1, &mesh->EBO);

  glBindVertexArray(mesh->VAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*shapeMesh->points) * shapeMesh->npoints * 3,
                 shapeMesh->points,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(*shapeMesh->triangles) * shapeMesh->ntriangles * 3,
                 shapeMesh->triangles,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(*shapeMesh->points), (GLvoid*)0);
    glEnableVertexAttribArray(0);
  }
}

void drawRenderableMesh(RenderableMesh *mesh)
{
  glBindVertexArray(mesh->VAO);
  glDrawElements(GL_TRIANGLES, mesh->mesh->ntriangles * 3, GL_UNSIGNED_SHORT, 0);
  glBindVertexArray(0);
}

void destroyRenderableMesh(RenderableMesh *mesh)
{
  GLuint buffers[3];
  buffers[0] = mesh->VAO;
  buffers[1] = mesh->VBO;
  buffers[2] = mesh->EBO;
  glDeleteBuffers(3, buffers);
}

struct Camera
{
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;

  GLfloat yaw;
  GLfloat pitch;
};

void initCamera(Camera* camera)
{
  camera->position = glm::vec3(0.f, 0.f, -5.f);
  camera->front = glm::vec3(0.f, 0.f, 1.f);
  camera->up = glm::vec3(0.f, 1.f, 0.f);

  camera->yaw = 90.f;
  camera->pitch = 0.f;
}

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
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  GLchar* vertexShaderSource =
    "#version 400 core\n"
    "layout (location = 0) in vec3 position;"
    "uniform mat4 model;"
    "uniform mat4 view;"
    "uniform mat4 projection;"
    "void main()"
    "{"
    " gl_Position = projection * view * model * vec4(position.x, position.y, position.z, 1.0);"
    "}";

  GLchar* fragmentShaderSource =
    "#version 400 core\n"
    "out vec4 color;"
    "void main()"
    "{"
    " color = vec4(0.7f, 0.7f, 0.7f, 1.0f);"
    "}";

  GLuint vertexShaderID;
  vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShaderID, 1, &vertexShaderSource, NULL);
  glCompileShader(vertexShaderID);

  GLuint fragmentShaderID;
  fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShaderID, 1, &fragmentShaderSource, NULL);
  glCompileShader(fragmentShaderID);

  GLuint shaderProgramID = glCreateProgram();
  glAttachShader(shaderProgramID, vertexShaderID);
  glAttachShader(shaderProgramID, fragmentShaderID);
  glLinkProgram(shaderProgramID);

  GLint success;
  glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    printf("failure");
  }

  glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &success);
  if(!success)
  {
    printf("failure");
  }

  glDeleteShader(vertexShaderID);
  glDeleteShader(fragmentShaderID);

  glm::mat4 model, view, projection;
  model = glm::rotate(model, 20.f, glm::vec3(1.f, 0.f, 0.f));
  //view = glm::translate(view, glm::vec3(0.f, 0.f, -5.f));
  projection = glm::perspective(45.f, (GLfloat)width / height, 0.1f, 100.f);

  Camera camera;
  initCamera(&camera);

  GLfloat mouseCoordsAtLastFrameStart[2];
  mouseCoordsAtLastFrameStart[0] = mouseCoords[0];
  mouseCoordsAtLastFrameStart[1] = mouseCoords[1];

  RenderableMesh cube;
  initRenderableMesh(&cube, par_shapes_create_cube());

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
    model = glm::rotate(model, ROTATION_SPEED * dT, glm::vec3(1.f, 1.f, 0.f));
    view = glm::lookAt(camera.position, camera.position + camera.front, camera.up);

    glClearColor(0.4f, 0.6f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    GLint modelLocation = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    GLint viewLocation = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    GLint projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    drawRenderableMesh(&cube);

    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
      printf("Error: %i", err);
    }

    glfwSwapBuffers(window);
  }

  par_shapes_free_mesh(cube.mesh);
  destroyRenderableMesh(&cube);

  glfwTerminate();

	return 0;
}
