R""(
#version 400 core
out vec3 VertexNormal;
out vec3 VertexPosition;
out vec3 VertexColor;
out vec2 UV;

layout (location = 0) in vec3 PositionVBO;
layout (location = 1) in vec3 NormalVBO;
layout (location = 2) in vec2 UVVBO;

uniform mat4 ModelMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ProjectionMatrix;
uniform vec3 ObjectColor;

void
main()
{
  gl_Position = ProjectionMatrix * ViewMatrix * ModelMatrix
                  * vec4(PositionVBO.x, PositionVBO.y, PositionVBO.z, 1.f);
  VertexNormal = mat3(transpose(inverse(ModelMatrix))) * NormalVBO;
  VertexPosition = vec3(ModelMatrix * vec4(PositionVBO, 1.f));
  VertexColor = ObjectColor;
  UV = UVVBO;
}
)"";
