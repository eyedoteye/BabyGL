struct RenderObject
{
  par_shapes_mesh* mesh;
  GLuint VAO, VBO, EBO, VBNO, VBUVO;
  glm::mat4 model;
  glm::vec3 color;
};

void updateRenderObject(RenderObject *renderableModel)
{
  glBindVertexArray(renderableModel->VAO);
  {
    glBindBuffer(GL_ARRAY_BUFFER, renderableModel->VBO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->points) * renderableModel->mesh->npoints * 3,
                 renderableModel->mesh->points,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderableModel->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->triangles) * renderableModel->mesh->ntriangles * 3,
                 renderableModel->mesh->triangles,
                 GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3,
                          GL_FLOAT, GL_FALSE,
                          3 * sizeof(*renderableModel->mesh->points), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, renderableModel->VBNO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->normals) * renderableModel->mesh->npoints * 3,
                 renderableModel->mesh->normals,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3,
                          GL_FLOAT, GL_FALSE,
                          3 * sizeof(*renderableModel->mesh->normals), (GLvoid*)0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, renderableModel->VBUVO);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(*renderableModel->mesh->tcoords) * renderableModel->mesh->npoints * 3,
                 renderableModel->mesh->tcoords,
                 GL_STATIC_DRAW);
    glVertexAttribPointer(2, 3,
                          GL_FLOAT, GL_FALSE,
                          2 * sizeof(*renderableModel->mesh->tcoords), (GLvoid*)0);
    glEnableVertexAttribArray(2);
  }
}

void initRenderObject(RenderObject *renderableModel, par_shapes_mesh *shapeMesh)
{
  renderableModel->mesh = shapeMesh;
  renderableModel->color = glm::vec3(.7f, .7f, .7f);

  par_shapes_unweld(shapeMesh, true);
  par_shapes_compute_normals(shapeMesh);

  glGenVertexArrays(1, &renderableModel->VAO);
  glGenBuffers(1, &renderableModel->VBO);
  glGenBuffers(1, &renderableModel->EBO);
  glGenBuffers(1, &renderableModel->VBNO);
  glGenBuffers(1, &renderableModel->VBUVO);

  updateRenderObject(renderableModel);
}

void drawRenderObject(RenderObject *renderableModel, GLint shaderProgramID)
{
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
  GLuint buffers[3];
  buffers[0] = renderableModel->VAO;
  buffers[1] = renderableModel->VBO;
  buffers[2] = renderableModel->EBO;
  glDeleteBuffers(3, buffers);
}
