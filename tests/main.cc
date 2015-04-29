//
// Created by Steven on 4/27/15.
//

#include <gtest/gtest.h>
#include "c_cases.h"

using namespace std;


TEST(MINUNIT, RunCTests)
{
  auto const minunit_res = c_run_all_tests();

  EXPECT_EQ(0, minunit_res) << "Test failed with message: " << minunit_res << "\n";

  RecordProperty("Minunit tests run", tests_run);
}


int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);

#ifdef WIN32
    // prevents visual studio from exiting early
    auto res = RUN_ALL_TESTS();
    getchar();
    return res;

#else
    return RUN_ALL_TESTS();
#endif

}
