#include "pch.h"
#include "../../src/OptimusBot/Utilities.h"

TEST(TestCaseName, TestName) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

using namespace OptimusBot::Utilities;

namespace UtilitiesTests
{

	TEST(RandomWithSingleDecimalPoint, ReturnsRandomNumberBetweenRange)
	{
		for (int min = 0; min < 1000; min++)
			for (int max = min+1; max < 1000; max++)
			{
				EXPECT_TRUE(RandomWithSingleDecimalPoint(min, max) >= 0.0);
				EXPECT_TRUE(RandomWithSingleDecimalPoint(min, max) >= min);
				EXPECT_TRUE(RandomWithSingleDecimalPoint(min, max) <= max);
			}
	}

	TEST(PlacePrudentOrders, ReturnsEmptyIfNumberOfOrdersSmallerThanOne)
	{
		//Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		const auto dummyLambda = [](double, double) {return std::optional<IDvfSimulator::OrderID>{}; };

		// Act & Assert
		EXPECT_TRUE(PlacePrudentOrders(wallet, bestOrder, 0, dummyLambda).empty());
	}

	TEST(PlacePrudentOrders, ReturnsEmptyNoOrdersWerePlaced)
	{
		//Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		const auto lambdaReturningEmpty = [](double, double) {return std::optional<IDvfSimulator::OrderID>{}; };

		// Act & Assert
		EXPECT_TRUE(PlacePrudentOrders(wallet, bestOrder, 5, lambdaReturningEmpty).empty());
	}

	TEST(PlacePrudentOrders, CallsPlaceOrderLambdaTwicePerIteration)
	{
		//Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		constexpr auto numberOfOrders{ 5 };
		auto counter{ 0 };
		const auto placeOrderMock = [&counter](double, double) {counter++;  return std::optional<IDvfSimulator::OrderID>{}; };

		// Act
		const auto _ = PlacePrudentOrders(wallet, bestOrder, numberOfOrders, placeOrderMock);

		// Assert 
		EXPECT_EQ(counter, 2*numberOfOrders);
	}

	TEST(PlacePrudentOrders, ReturnsAnArrayOfOrders)
	{
		//Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		const auto placeOrderMock = [](double, double) {return std::optional<IDvfSimulator::OrderID>{rand()}; };

		// Act & Assert
		EXPECT_EQ(PlacePrudentOrders(wallet, bestOrder, 5, placeOrderMock).size(), 10);
	}

	TEST(PlacePrudentOrders, AllOrdersAreWithin5PercentOfBestBidAndBestAsk)
	{
		//Arrange
		const Wallet wallet(1.0, 10.0);
		constexpr auto bestBid = 50;
		constexpr auto bestAsk = 51;
		const BestOrder bestOrder(bestBid, bestAsk);
		const auto placeOrderMock = [](double, double) {return std::optional<IDvfSimulator::OrderID>{rand()}; };

		// Act
		const auto orders = PlacePrudentOrders(wallet, bestOrder, 5, placeOrderMock);

		// Assert
		for (const auto& order : orders)
		{
			if (order.Side == OrderSide::BID)
				EXPECT_TRUE(0.95 * bestBid <= order.Price && order.Price <= bestBid);
			else
				EXPECT_TRUE(bestAsk <= order.Price && order.Price <= 1.05*bestAsk);
		}
	}
}

