R""(
#version 400 core
out vec3 vertexNormal;
out vec3 vertexPosition;
out vec3 vertexColor;
out vec2 textureCoords;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 uvCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 objectColor;

void main()
{
  gl_Position = projection * view * model
                  * vec4(position.x, position.y, position.z, 1.f);
  vertexNormal = mat3(transpose(inverse(model))) * normal;
  vertexPosition = vec3(model * vec4(position, 1.f));
  vertexColor = objectColor;
  textureCoords = uvCoords;
}

)"";
