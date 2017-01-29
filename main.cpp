#include <stdio.h>

#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct plane
{
  GLfloat vertices[12];
  GLuint indices[12];
};

static plane squarePlane = {
  {
    -0.5f, -0.5f, 0.0f,
    -0.5f,  0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.5f,  0.5f, 0.0f
  },
  {
    0, 1, 2,
    2, 1, 3
  }
};

int main()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
  view = glm::translate(view, glm::vec3(0.f, 0.f, -5.f));
  projection = glm::perspective(45.f, (GLfloat)width / height, 0.1f, 100.f);

  GLuint VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(squarePlane.vertices), squarePlane.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squarePlane.indices), squarePlane.indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
  }

  GLfloat dT = 0.f;
  GLfloat timeAtLastFrameStart = 0.f;
  while(!glfwWindowShouldClose(window))
  {
    GLfloat timeNow = (GLfloat)glfwGetTime();
    dT = timeNow - timeAtLastFrameStart;
    timeAtLastFrameStart = timeNow;

    glfwPollEvents();

#define ROTATION_SPEED 1.f
    model = glm::rotate(model, ROTATION_SPEED * dT, glm::vec3(1.f, 1.f, 0.f));


    glClearColor(0.4f, 0.6f, 0.2f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shaderProgramID);

    GLint modelLocation = glGetUniformLocation(shaderProgramID, "model");
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(model));
    GLint viewLocation = glGetUniformLocation(shaderProgramID, "view");
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, glm::value_ptr(view));
    GLint projectionLocation = glGetUniformLocation(shaderProgramID, "projection");
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glfwSwapBuffers(window);
  }

  glfwTerminate();

	return 0;
}
