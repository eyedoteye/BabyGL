struct render_object
{
  int Type;
  par_shapes_mesh* Mesh;
  GLuint VAO, EBO;
  GLuint PositionVBO, NormalVBO, UVVBO;
  glm::mat4 ModelMatrix;
  glm::vec3 Color;
};

void
UpdateRenderObject(render_object *Renderable)
{
  glBindVertexArray(Renderable->VAO);
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Renderable->EBO);
    {
      glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                   sizeof(*Renderable->Mesh->triangles)
                     * Renderable->Mesh->ntriangles * 3,
                   Renderable->Mesh->triangles,
                   GL_STATIC_DRAW);
    }
    glBindBuffer(GL_ARRAY_BUFFER, Renderable->PositionVBO);
    {
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(*Renderable->Mesh->points) * 3
                     * Renderable->Mesh->npoints,
                   Renderable->Mesh->points,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
                            sizeof(*Renderable->Mesh->points) * 3,
                            (GLvoid*)0);
      glEnableVertexAttribArray(0);
    }
    glBindBuffer(GL_ARRAY_BUFFER, Renderable->NormalVBO);
    {
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(*Renderable->Mesh->normals) * 3
                     * Renderable->Mesh->npoints,
                   Renderable->Mesh->normals,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
                            sizeof(*Renderable->Mesh->normals) * 3,
                            (GLvoid*)0);
      glEnableVertexAttribArray(1);
    }
    glBindBuffer(GL_ARRAY_BUFFER, Renderable->UVVBO);
    {
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(*Renderable->Mesh->tcoords) * 2
                     * Renderable->Mesh->npoints,
                   Renderable->Mesh->normals,
                   GL_STATIC_DRAW);
      glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE,
                            sizeof(*Renderable->Mesh->tcoords) * 2,
                            (GLvoid*)0);
      glEnableVertexAttribArray(2);
    } glBindBuffer(GL_ARRAY_BUFFER, 0);
  }
}

// Todo: Add a flyweight system that inits with defaults
// if any parameters are missing.
void
InitRenderObject(render_object *Renderable)
{
  glGenVertexArrays(1, &Renderable->VAO);
  glGenBuffers(1, &Renderable->EBO);
  glGenBuffers(1, &Renderable->PositionVBO);
  glGenBuffers(1, &Renderable->NormalVBO);
  glGenBuffers(1, &Renderable->UVVBO);

  UpdateRenderObject(Renderable);
}

void
DrawRenderObject(render_object *Renderable, GLint ShaderID)
{
  glUniform1i(glGetUniformLocation(ShaderID, "ObjectType"),
    Renderable->Type); 
  glUniformMatrix4fv(glGetUniformLocation(ShaderID, "ModelMatrix"), 1, GL_FALSE,
    glm::value_ptr(Renderable->ModelMatrix));
  glUniform3fv(glGetUniformLocation(ShaderID, "ObjectColor"), 1,
    glm::value_ptr(Renderable->Color));

  glBindVertexArray(Renderable->VAO);
  glDrawElements(GL_TRIANGLES, Renderable->Mesh->ntriangles * 3,
    GL_UNSIGNED_SHORT, 0);
  glBindVertexArray(0);
}

void destroyRenderObject(render_object *Renderable)
{
  // Todo
  // Note: Not currently needed as objects are currently only destroyed at
  // program exit.
}
