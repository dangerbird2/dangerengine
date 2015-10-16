/**
 * @file ${FILE}
 * @brief 
 * @license ${LICENSE}
 * Copyright (c) 10/8/15, Steven
 * 
 **/
#include "slsEntityDraw.h"
#include <state/slsEntity.h>
#include <renderer/slsshader.h>


void sls_entity_draw(slsEntity *self, double dt, slsAppState *state)
{
  if (self->mesh) {

    sls_msg(self->mesh, pre_draw, self->shader->program, dt);
    sls_msg(self->mesh, draw, dt);
    sls_msg(self->mesh, post_draw, self->shader->program, dt);

  }
}

void sls_drawable_transform(slsEntity *self, slsAppState *state, double dt)
{


  if (self->texture) {
    sls_msg(self->texture, bind);
  }

  if (self->shader) {
    sls_matrix_glbind(&state->model_view,
                      self->shader->program,
                      self->shader->uniforms.model_view,
                      self->shader->uniforms.normal_map);
  }

}