/**
 * @file slscontext.c
 * @brief
 *
 * Copyright (c) 2015, Steven Shea
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of Steven Shea.
**/
#include "sls-gl.h"

#include "slscontext.h"
#include "slsutils.h"


#include <stdlib.h>
#include <stdio.h>
#include <math/math-types.h>
#include <assert.h>

#include "contexthandlers.h"
#include <apr-1/apr_errno.h>


#ifdef EMSCRIPTEN
#   include <emscripten.h>
#endif //EMSCRIPTEN


struct slsContext_p {
  clock_t last;
  clock_t dt_acc;
  slsIPoint last_size;
};



/*----------------------------------------*
 * slsContext default method prototypes
 *----------------------------------------*/
void sls_context_run(slsContext *self) SLS_NONNULL(1);

void sls_context_iter(slsContext *self) SLS_NONNULL(1);

void sls_context_resize(slsContext *self, int x, int y) SLS_NONNULL(1);

void sls_context_update(slsContext *self, double dt) SLS_NONNULL(1);

void sls_context_display(slsContext *self, double dt) SLS_NONNULL(1);

slsContext *sls_context_init(slsContext *self,
                             char const *caption,
                             size_t width,
                             size_t height) SLS_NONNULL(1);

void sls_context_setup(slsContext *self) SLS_NONNULL(1);

void sls_context_teardown(slsContext *self) SLS_NONNULL(1);

void sls_context_dtor(slsContext *self) SLS_NONNULL(1);


void sls_context_handle_event(slsContext *self, SDL_Event const *e);

void sls_context_setupstate(slsContext *pContext) SLS_NONNULL(1);

/*----------------------------------------*
 * slsContext static prototype
 *----------------------------------------*/
static const slsContext sls_context_proto = {
    .init = sls_context_init,
    .dtor = sls_context_dtor,

    .setup = sls_context_setup,
    .teardown = sls_context_teardown,

    .run = sls_context_run,
    .iter = sls_context_iter,
    .resize = sls_context_resize,

    .update = sls_context_update,
    .display = sls_context_display,

    .handle_event = sls_context_handle_event,

    .is_running = SLS_FALSE,
    .interval = 50,
    .priv = NULL,
    .window = NULL,
    .state = NULL,
    .data = NULL,

    .pool = NULL,
    .tmp_pool = NULL
};


/*----------------------------------------*
 * slsContext class functions
 *----------------------------------------*/

slsContext const *sls_context_class()
{
  return &sls_context_proto;
}

slsContext *sls_context_new(char const *caption, size_t width, size_t height)
{

  slsContext *self = sls_objalloc(sls_context_class(), sizeof(slsContext));


  return self->init(self, caption, width, height);
}


/*----------------------------------------*
 * slsContext instance methods
 *----------------------------------------*/

slsContext *sls_context_init(slsContext *self,
                             char const *caption,
                             size_t width,
                             size_t height)
{

  *self = *sls_context_class();
  uint32_t window_flags;
  GLenum glew;

  // initialize libraries if not active
  if (!sls_is_active()) {
    bool res = sls_init();
    sls_check(res, "initialization failed!");
  }
  apr_status_t res = apr_pool_create(&self->pool, NULL);
  char buffer[20];
  sls_check(res == APR_SUCCESS,
            "pool creation failed: %s", apr_strerror(res, buffer, 20));

  // create sdl window

  window_flags =
      SDL_WINDOW_ALLOW_HIGHDPI |
      SDL_WINDOW_RESIZABLE;
  self->window = SDL_CreateWindow(caption,
                                  SDL_WINDOWPOS_CENTERED,
                                  SDL_WINDOWPOS_CENTERED,
                                  (int) width,
                                  (int) height,
                                  window_flags);

  sls_check(self->window, "window creation failed");

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  self->gl_context = SDL_GL_CreateContext(self->window);


  glewExperimental = GL_TRUE;
  glew = glewInit();
  if (glew != GLEW_OK) {
    sls_log_err("glew error: %s", glewGetErrorString(glew));
    self->is_running = SLS_FALSE;
  }
  sls_log_info("\nglew version %s\n"
                   "gl version %s",
               glewGetString(GLEW_VERSION),
               glGetString(GL_VERSION));


  // allocate and initialize private members

  sls_checkmem(apr_pool_create(&self->tmp_pool, self->pool) == APR_SUCCESS);

  self->priv = apr_pcalloc(self->pool, sizeof(slsContext_p));
  sls_checkmem(self->priv);


  return self;

  /// exception
  error:

  sls_log_err("sdl error: %s", SDL_GetError());
  if (self->dtor) {
    sls_msg(self, dtor);
  }
  free(self);
  return NULL;
}

void sls_context_dtor(slsContext *self)
{
  if (self->pool) {
    apr_pool_destroy_debug(self->pool, __FILE__);
  }
  free(self);
}


void sls_context_run(slsContext *self)
{
  if (!self->priv) { return; }
  self->priv->last = clock();
  self->priv->dt_acc = 0;

  self->is_running = SLS_TRUE;

  sls_msg(self, setup);

  self->frame_n = 0;


  // setup render size
  int x, y, w, h;

  SDL_GetWindowSize(self->window, &w, &h);

  sls_msg(self, resize, w * 2, h * 2);


  sls_msg(self, display, 0.0);
  while (self->is_running) {
    sls_msg(self, iter);
  }

  sls_msg(self, teardown);
}

void sls_emscripten_loop(void *vctx)
{
  slsContext *ctx = vctx;
  sls_msg(ctx, iter);
}

void sls_context_iter(slsContext *self)
{
  sls_context_pollevents(self);

  if (!self->priv) {
    return;
  }


  clock_t now = clock();
  slsContext_p *priv = self->priv;
  priv->dt_acc += now - priv->last;
  priv->last = now;


  if (priv->dt_acc > self->interval) {


    double dt = priv->dt_acc / (double) CLOCKS_PER_SEC;
    priv->dt_acc = 0;


    sls_msg(self, update, dt);

    glClear(GL_COLOR_BUFFER_BIT |
            GL_DEPTH_BUFFER_BIT);

    sls_msg(self, display, dt);

    SDL_GL_SwapWindow(self->window);

    // reset input state after update interval
    sls_appstate_clearinput(self->state);

    if (self->frame_n % 5 == 0) {
      apr_pool_clear(self->tmp_pool);
    }

    self->frame_n++;


  }

}


void sls_context_resize(slsContext *self, int x, int y)
{
  glViewport(0, 0, (int) x, (int) y);

  if (self->state) {
    sls_appstate_resize(self->state, x, y);
  }

}


void sls_context_update(slsContext *self, double dt)
{
  sls_appstate_update(self->state, dt);
}

void sls_context_display(slsContext *self, double dt)
{
  sls_appstate_display(self->state, dt);
}

void sls_context_setup(slsContext *self)
{
  if (!self->priv) {
    assert(0);
    return;
  }

  slsContext_p *priv = self->priv;


  sls_context_setupstate(self);


  sls_log_info("openGL version %s", glGetString(GL_VERSION));

  // setup opengl pipeline

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glEnable(GL_POINT_SIZE);
  glEnable(GL_POINT_SPRITE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  int x, y;
  SDL_GetWindowSize(self->window, &x, &y);

  sls_msg(self, resize, x * 2, y * 2);


}

void sls_context_setupstate(slsContext *self)
{
  if (!self->state) {
    self->state = apr_pcalloc(self->pool, sizeof(slsAppState));
  }
  self->state = sls_appstate_init(self->state, self->pool);
  self->state->context = self;

}

void sls_context_pollevents(slsContext *self)
{

  SDL_Event e;

  if (self->handle_event) {
    while (SDL_PollEvent(&e)) {
      self->handle_event(self, &e);
    }
  }
}

static inline void _sls_context_windowevent(slsContext *self, SDL_WindowEvent const *we)
{
  switch (we->event) {
    case SDL_WINDOWEVENT_RESIZED: {
      int w = we->data1 * 2, h = we->data2 * 2;
      sls_msg(self, resize, w, h);
    }
      break;
    default:
      break;
  }
}

void sls_context_handle_event(slsContext *self, SDL_Event const *e)
{

  switch (e->type) {
    case SDL_QUIT:
      self->is_running = SLS_FALSE;
      break;
    case SDL_WINDOWEVENT:
      _sls_context_windowevent(self, &e->window);
      break;
    default:
      break;
  }
  // pass event to
  if (self->state) {
    sls_appstate_handle_input(self->state, e);
  }
}

void sls_context_teardown(slsContext *self)
{
  self->state = sls_appstate_dtor(self->state);
}

#ifndef __EMSCRIPTEN__

int sls_get_glversion()
{
  int
      major = 0,
      minor = 0,
      major_mul = 100,
      minor_mul = 10,
      full;

  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  full = (major * major_mul) + (minor * minor_mul);
  return full;
}

#else
int sls_get_glversion()
{
  // emscripten uses webgl 1.0.0
  const int webgl_default_version = 100;
  return webgl_default_version;
}

#endif
