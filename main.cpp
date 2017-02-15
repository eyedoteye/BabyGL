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

//ToDo:
// Sliders
//   Draw Rectangle
//   Draw Text!?!
// Draw Alpha Blended, Scaled Quad

struct SliderGUI{
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
    "out vec4 fragmentColor;"
    "in vec2 textureCoords;"

    "uniform sampler2D gPosition;"
    "uniform sampler2D gNormal;"
    "uniform sampler2D gColor;"
    "uniform vec3 viewPosition;"
    "uniform vec3 lightPosition;"
    "uniform vec3 lightColor;"

    "uniform float diffuseImpact;"
    "uniform float specularImpact;"

    "void main()"
    "{"
    " vec3 objectColor = texture2D(gColor, textureCoords).rgb;"

    " vec3 ambient = 0.1f * lightColor;"

    " vec3 normal = texture2D(gNormal, textureCoords).rgb;"
    " vec3 vertexPosition = texture2D(gPosition, textureCoords).rgb;"
    " vec3 lightDirection = normalize(lightPosition - vertexPosition);"
    " vec3 diffuse = diffuseImpact * max(dot(normal, lightDirection), 0.f) * lightColor;"

    " vec3 viewDirection = normalize(viewPosition - vertexPosition);"
    " vec3 reflectDirection = reflect(-lightDirection, normal);"
    " float spec = pow(max(dot(viewDirection, reflectDirection), 0.f), 64);"
    " vec3 specular = specularImpact * spec * lightColor;"
    " if(normal == vec3(0.f))"
    "  fragmentColor = vec4(.4f, .6f, .2f, 1.f);"
    " else"
    "  fragmentColor = vec4((ambient + diffuse + specular) * objectColor, 1.f);"
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
    " float t = gs_in[0].t * .96f;"
    " float height = gs_in[0].dimensions.y;"
    " buildSquare(gl_in[0].gl_Position, t, height);"
    " buildSquare(gl_in[0].gl_Position + vec4(t + 0.01f, vec3(0.f)), 0.02f, height);"
    " buildSquare(gl_in[0].gl_Position + vec4(t + 0.04f, vec3(0.f)), 1.f - t - 0.04f, height);"
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
    P2F(5, SCREEN_WIDTH), 5.f / SCREEN_HEIGHT * 2,
    P2F(5, SCREEN_WIDTH),

    P2F(0, SCREEN_WIDTH), P2F(SCREEN_HEIGHT - 6, SCREEN_HEIGHT),
    P2F(5, SCREEN_WIDTH), 5.f / SCREEN_HEIGHT * 2,
    P2F(5, SCREEN_WIDTH)
  };

  float sliderTs[] = {
    .5f, .3f
  };

  SliderGUI sliderGUI = {};
  initSliderGUI(&sliderGUI, sliderStaticInfo, sliderTs, 2);

  GLuint perlinNoiseTextureID;
  glGenTextures(1, &perlinNoiseTextureID);
  generate2DPerlinNoise(perlinNoiseTextureID, 420);

  Camera camera;
  initCamera(&camera, (GLfloat)width, (GLfloat)height);

  glm::vec3 lightPosition = glm::vec3(0.f, 5.f, -2.f);
  glm::vec3 lightColor = glm::vec3(1.f);

  RenderObject shape;
  initRenderObject(&shape,
                   par_shapes_create_parametric_sphere(32, 32));
  shape.model = glm::translate(shape.model, glm::vec3(0.f, 0.f, 0.f));

  RenderObject shape2;
  initRenderObject(&shape2,
                   par_shapes_create_parametric_sphere(32,32));
  shape2.model = glm::translate(shape2.model, glm::vec3(0.f, 1.f, -3.f));
  shape2.color = glm::vec3(.2f, 1.f, 1.f);

  float specularImpact = 0;
  float diffuseImpact = 0;

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
    if(keys[GLFW_KEY_3])
      specularImpact -= SLIDERSPEED * dT;
    if(keys[GLFW_KEY_4])
      specularImpact += SLIDERSPEED * dT;

    if(keys[GLFW_KEY_1])
      diffuseImpact -= SLIDERSPEED * dT;
    if(keys[GLFW_KEY_2])
      diffuseImpact += SLIDERSPEED * dT;

    if(diffuseImpact < 0)
      diffuseImpact = 0;
    if(diffuseImpact > 255)
      diffuseImpact = 255;

    if(specularImpact < 0)
      specularImpact = 0;
    if(specularImpact > 255)
      specularImpact = 255;

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
    GLint lightPositionLocation= glGetUniformLocation(lPassShader.shaderProgramID, "lightPosition");
    glUniform3fv(lightPositionLocation, 1, glm::value_ptr(lightPosition));
    GLint lightColorLocation = glGetUniformLocation(lPassShader.shaderProgramID, "lightColor");
    glUniform3fv(lightColorLocation, 1, glm::value_ptr(lightColor));

    GLint diffuseImpactLocation = glGetUniformLocation(lPassShader.shaderProgramID, "diffuseImpact");
    glUniform1f(diffuseImpactLocation, diffuseImpact / 255.f);
    GLint specularImpactLocation = glGetUniformLocation(lPassShader.shaderProgramID, "specularImpact");
    glUniform1f(specularImpactLocation, specularImpact / 255.f);

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glUseProgram(guiShader.shaderProgramID);

    sliderTs[0] = diffuseImpact / 255.f;
    sliderTs[1] = specularImpact / 255.f;
    glBindBuffer(GL_ARRAY_BUFFER, sliderGUI.vboTs);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(float) * 2,
                 sliderTs,
                 GL_DYNAMIC_DRAW);
    glBindVertexArray(sliderGUI.VAO);
    glDrawArrays(GL_POINTS, 0, 2);
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
