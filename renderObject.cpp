struct RenderObject
{
  int objectType;
  par_shapes_mesh* mesh;
  GLuint VAO, EBO;
  GLuint vboPositions, vboNormals, vboUVCoords;
  glm::mat4 model;
  glm::vec3 color;
};

void updateRenderObject(RenderObject *renderableModel)
{
  glBindVertexArray(renderableModel->VAO);
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderableModel->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->triangles)
                   * renderableModel->mesh->ntriangles * 3,
                 renderableModel->mesh->triangles,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, renderableModel->vboPositions);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->points) * 3
                   * renderableModel->mesh->npoints,
                 renderableModel->mesh->points,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                          sizeof(*renderableModel->mesh->points) * 3,
                          (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, renderableModel->vboNormals);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->normals) * 3
                   * renderableModel->mesh->npoints,
                 renderableModel->mesh->normals,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                          sizeof(*renderableModel->mesh->normals) * 3,
                          (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, renderableModel->vboUVCoords);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->tcoords) * 2
                   * renderableModel->mesh->npoints,
                 renderableModel->mesh->normals,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                          sizeof(*renderableModel->mesh->tcoords) * 2,
                          (GLvoid*)0);
    glEnableVertexAttribArray(2);
  }
}

// Note: Add a flyweight system that inits with defaults
// if any parameters are missing.
void initRenderObject(RenderObject *renderableModel)
{
  glGenVertexArrays(1, &renderableModel->VAO);
  glGenBuffers(1, &renderableModel->EBO);
  glGenBuffers(1, &renderableModel->vboPositions);
  glGenBuffers(1, &renderableModel->vboNormals);
  glGenBuffers(1, &renderableModel->vboUVCoords);

  updateRenderObject(renderableModel);
}

void drawRenderObject(RenderObject *renderableModel, GLint shaderProgramID)
{
  GLint objectTypeLocation = glGetUniformLocation(shaderProgramID, "objectType");
  glUniform1i(objectTypeLocation, renderableModel->objectType); 
  GLint modelLocation = glGetUniformLocation(shaderProgramID, "model");
  glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(renderableModel->model));
  GLint objectColorLocation = glGetUniformLocation(shaderProgramID, "objectColor");
  glUniform3fv(objectColorLocation, 1, glm::value_ptr(renderableModel->color));

  glBindVertexArray(renderableModel->VAO);
  glDrawElements(GL_TRIANGLES, renderableModel->mesh->ntriangles * 3, GL_UNSIGNED_SHORT, 0);
  glBindVertexArray(0);
}

void destroyRenderObject(RenderObject *renderableModel)
{
  // Todo
}
