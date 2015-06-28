//
// Created by Steven on 6/22/15.
//

#include <gtest/gtest.h>
#include "../src/dangerengine.h"
#include <string>
#include "test-utils.h"

using namespace std;
using namespace testing;

class RenderTests: public Test {
protected:

  slsContext *ctx = nullptr;

  RenderTests()
  {
    sls_init();
    ctx = sls_context_new("title", 480, 480);
    EXPECT_TRUE(ctx) << "ctx is null";
    sls_msg(ctx, setup);

  }

  ~RenderTests()
  {
    sls_msg(ctx, dtor);
    sls_terminate();


    Test::TearDown();
  }

};


TEST_F(RenderTests, ContextCreation)
{
  ASSERT_TRUE(ctx) << "context is null";
}

TEST_F(RenderTests, ShaderRead)
{

  auto vs_path = string("resources/shaders/default.vert");
  auto fs_path = string("resources/shaders/default.frag");
  auto program = sls_create_program(vs_path.c_str(), fs_path.c_str());

  auto res = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &res);
  EXPECT_TRUE(res);

  glDeleteProgram(program);
}
