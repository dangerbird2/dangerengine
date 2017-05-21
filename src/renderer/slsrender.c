#include "slsrender.h"

static void scene_setup(slsRendererGL* self);

slsRendererGL* sls_renderer_init(slsRendererGL* self, int width, int height)
{
  const size_t n_render_items = 0x100;
  scene_setup(self);
  sls_renderer_resize(self, width, height);

  return self;
}

static void scene_setup(slsRendererGL* self)
{
  kmMat4Translation(&self->root_modelview, 0.f, 0.f, -5.f);

  return;
error:
  exit(EXIT_FAILURE);
}

slsRendererGL* sls_renderer_dtor(slsRendererGL* self)
{
  return self;
}

void sls_renderer_display(slsRendererGL* self)
{
  if (self->active_mesh && self->active_shader) {
    sls_renderer_draw_mesh(
      self, self->active_mesh, self->active_shader, &self->root_modelview);
  }
}

void sls_renderer_resize(slsRendererGL* self, int width, int height)
{
  self->width = width;
  self->height = height;
  float aspect = width / (float)height;
  kmMat4PerspectiveProjection(&self->projection, 45.f, aspect, 0.01f, 1000.f);
}

void sls_renderer_draw_mesh(slsRendererGL* self,
                            slsMesh* mesh,
                            slsShader* shader,
                            kmMat4* model_view)
{
  glUseProgram(shader->program);
  glUniformMatrix4fv(glGetUniformLocation(shader->program, "projection"),
                     1,
                     false,
                     (float*)&self->projection);
  glUniformMatrix4fv(glGetUniformLocation(shader->program, "model_view"),
                     1,
                     false,
                     (float*)&self->root_modelview);

  bool is_valid = glIsProgram(shader->program) && glIsBuffer(mesh->ibo) &&
                  glIsBuffer(mesh->vbo) && glIsVertexArray(mesh->vao);
  if (!is_valid)
    return;
  sls_mesh_bindbuffers(mesh);
  {
    size_t elements = mesh->indices.length;
    sls_drain_glerrors();
    // glDrawElements(GL_TRIANGLES, (int) elements, GL_UNSIGNED_INT, NULL);
    sls_debug_glerrors();
  }
  sls_mesh_unbind();

  // calculate inverse and normal matrices
}
