struct camera
{
  glm::mat4 Projection;
  glm::mat4 View;

  glm::vec3 Position;
  glm::vec3 Front;
  glm::vec3 Up;

  GLfloat Yaw;
  GLfloat Pitch;
};

void
InitCamera(camera* Camera, GLfloat Width, GLfloat Height)
{
  Camera->Projection = glm::perspective(45.f, Width / Height, 0.1f, 100.f);

  Camera->Position = glm::vec3(0.f, 0.f, -5.f);
  Camera->Front = glm::vec3(0.f, 0.f, 1.f);
  Camera->Up = glm::vec3(0.f, 1.f, 0.f);

  Camera->Yaw = 90.f;
  Camera->Pitch = 0.f;
}

void
UpdateCamera(
  camera* Camera,
  GLfloat XOffset,
  GLfloat YOffset,
  bool* Keys,
  float Dt)
{
  if(Keys[GLFW_KEY_W])
    Camera->Position += Camera->Front * Dt;
  if(Keys[GLFW_KEY_S])
    Camera->Position -= Camera->Front * Dt;
  if(Keys[GLFW_KEY_A])
    Camera->Position -=
      glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Dt;
  if(Keys[GLFW_KEY_D])
    Camera->Position +=
      glm::normalize(glm::cross(Camera->Front, Camera->Up)) * Dt;
  if(Keys[GLFW_KEY_SPACE])
    Camera->Position += Camera->Up * Dt;
  if(Keys[GLFW_KEY_LEFT_SHIFT])
    Camera->Position -= Camera->Up * Dt;

  Camera->Pitch -= YOffset * 0.05f;
  Camera->Yaw += XOffset * 0.05f;

  Camera->Pitch = Camera->Pitch > 89.f ? 89.f : Camera->Pitch;
  Camera->Pitch = Camera->Pitch < -89.f ? -89.f : Camera->Pitch;

  glm::vec3 Front;
  Front.x = cos(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
  Front.y = sin(glm::radians(Camera->Pitch));
  Front.z = sin(glm::radians(Camera->Yaw)) * cos(glm::radians(Camera->Pitch));
  Camera->Front = glm::normalize(Front);

  Camera->View =
    glm::lookAt(Camera->Position, Camera->Position + Camera->Front, Camera->Up);
}
