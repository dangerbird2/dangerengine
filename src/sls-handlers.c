//
// Created by Steven on 5/2/15.
//

#include "sls-handlers.h"
#include "sls-gl.h"
#include "slsutils.h"
#include <stdlib.h>
#include <stdbool.h>

static slsBool sls_active_flag = SLS_FALSE;
slsContext *sls_active_context = NULL;

void sls_error_cback(int, const char *);

void sls_mouse(GLFWwindow *, int, int, int);

void sls_window_resize(GLFWwindow *win, int x, int y);


void sls_bind_context(slsContext *ctx)
{
  sls_unbind_context();

  glfwSetErrorCallback(sls_error_cback);
  glfwSetWindowSizeCallback(ctx->window, sls_window_resize);


  sls_active_context = ctx;
  glfwMakeContextCurrent(ctx->window);

#ifndef SLS_NOGLEW
  {
    GLenum err = glew_init();
  }
#endif

  return;
}

void sls_unbind_context(void)
{
  if (sls_active_context) {
    glfwMakeContextCurrent(NULL);
    sls_active_context = NULL;
  }
}


void sls_window_resize(GLFWwindow *win, int x, int y)
{
  if (sls_active_context) {
    sls_msg(sls_active_context, resize, (size_t) x, (size_t) y);
  }
}


bool sls_init(void)
{
  sls_check(!sls_active_flag, "runtime is already active!");
  sls_check(glfwInit(), "glfw Init failed");

  sls_active_flag = SLS_TRUE;

  atexit(sls_terminate);

  return true;
  error:
  return false;
}

void sls_terminate(void)
{
  if (!sls_active_flag) {
    return;
  }

  glfwTerminate();
  sls_active_flag = false;
}

void sls_error_cback(int i, char const *string)
{

}

bool sls_is_active(void)
{
  return sls_active_flag;
}
