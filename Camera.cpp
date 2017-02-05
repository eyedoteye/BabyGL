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

void updateCamera(Camera* camera, GLfloat xOffset, GLfloat yOffset, bool* keys, float dT)
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
