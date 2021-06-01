#include "pch.h"
#include "../../src/OptimusBot/Utilities.h"

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

namespace UtilitiesTests
{
	TEST(MakeRandomOrder, ReturnsAnOrder) 
	{
		EXPECT_EQ(OptimusBot::MakeRandomOrder(), 1);
	}
}

