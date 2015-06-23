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

  virtual void SetUp()
  {
    Test::SetUp();

    sls::silence_stderr([this](){
      ctx = sls_context_new("title", 10, 10);
      sls_msg(ctx, setup);
    });

  }

  virtual void TearDown()
  {
    sls_msg(ctx, dtor);


    Test::TearDown();
  }

};


TEST_F(RenderTests, ContextCreation)
{
  ASSERT_NE(nullptr, ctx);
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