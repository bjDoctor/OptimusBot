#include "pch.h"
#include "../../src/OptimusBot/Utilities.h"

using namespace OptimusBot::Utilities;
using namespace OptimusBot::Types;

namespace UtilitiesTests
{
	TEST(Random, ReturnsRandomNumberBetweenRange)
	{
		// Using the "property-based testing" appreach here:
		// generating a lot of inputs and validating that a property holds
		for (int min = 0; min < 1000; min++)
			for (int max = min+1; max < 1000; max++)
			{
				EXPECT_TRUE(Random(min, max) >= 0.0);
				EXPECT_TRUE(Random(min, max) >= min);
				EXPECT_TRUE(Random(min, max) <= max);
			}
	}

	TEST(PlacePrudentOrders, ReturnsEmptyIfNumberOfOrdersSmallerThanOne)
	{
		// Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		const auto dummyLambda = [](double, double) {return std::optional<IDvfSimulator::OrderID>{}; };

		// Act & Assert
		EXPECT_TRUE(PlacePrudentOrders(wallet, bestOrder, 0, dummyLambda).empty());
	}

	TEST(PlacePrudentOrders, ReturnsEmptyNoOrdersWerePlaced)
	{
		// Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		const auto lambdaReturningEmpty = [](double, double) {return std::optional<IDvfSimulator::OrderID>{}; };

		// Act & Assert
		EXPECT_TRUE(PlacePrudentOrders(wallet, bestOrder, 5, lambdaReturningEmpty).empty());
	}

	TEST(PlacePrudentOrders, CallsPlaceOrderLambdaTwicePerIteration)
	{
		// Arrange
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
		// Arrange
		const Wallet wallet(1.0, 10.0);
		const BestOrder bestOrder(50.0, 51.0);
		const auto placeOrderMock = [](double, double) {return std::optional<IDvfSimulator::OrderID>{rand()}; };

		// Act & Assert
		EXPECT_EQ(PlacePrudentOrders(wallet, bestOrder, 5, placeOrderMock).size(), 10);
	}

	TEST(PlacePrudentOrders, AllOrdersAreWithin5PercentOfBestBidAndBestAsk)
	{
		// Arrange
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

	TEST(ExtractBestOrder, GetsTheBestBidAskPairFromSortedOrderBook)
	{
		// Arrange 
		std::vector<std::pair<double, double>> orderBook{ {1.0, 1.0}, {2.0, 1.0}, {3.0, 1.0}, {4.0, -1.0}, {5.0, -1.0}, {6.0, -1.0} };

		// Act
		const auto result = ExtractBestOrder(orderBook);

		// Assert
		ASSERT_TRUE(result);
		EXPECT_EQ(3.0, result.value().Bid);
		EXPECT_EQ(4.0, result.value().Ask);
	}

	TEST(ExtractBestOrder, GetsTheBestBidAskPairFromUnsortedOrderBook)
	{
		// Arrange 
		std::vector<std::pair<double, double>> orderBook{ {2.0, 1.0}, {3.0, 1.0}, {1.0, 1.0}, {5.0, -1.0}, {6.0, -1.0}, {4.0, -1.0} };

		// Act
		const auto result = ExtractBestOrder(orderBook);

		// Assert
		ASSERT_TRUE(result);
		EXPECT_EQ(3.0, result.value().Bid);
		EXPECT_EQ(4.0, result.value().Ask);
	}

	TEST(EraseFilledOrders, ErasesOrdersWithBidPriceGreaterThanTheBestBid)
	{
		// Arrange
		const BotOrder bid { OrderSide::BID, {}, 10.0, {} };
		const BotOrder ask { OrderSide::ASK, {}, 14.0, {} };
		std::multiset<BotOrder> orders{ bid, ask, bid, ask  };
		const BestOrder bestOrder{ 9.0, 12.0 };

		// Act
		const auto result = EraseFilledOrders(orders, bestOrder);

		// Assert
		ASSERT_EQ(result.size(), 2);
		ASSERT_EQ(result.begin()->Side, bid.Side);
		ASSERT_EQ(result.begin()->OrderId, bid.OrderId);
		ASSERT_EQ(result.begin()->Price, bid.Price);
		ASSERT_EQ(result.begin()->Volume, bid.Volume);

		ASSERT_EQ(orders.size(), 2);
		ASSERT_EQ(orders.begin()->Side, ask.Side);
		ASSERT_EQ(orders.begin()->OrderId, ask.OrderId);
		ASSERT_EQ(orders.begin()->Price, ask.Price);
		ASSERT_EQ(orders.begin()->Volume, ask.Volume);
	}

	TEST(EraseFilledOrders, ErasesOrdersWithAskPriceSmallerThanTheBestAsk)
	{
		// Arrange
		const BotOrder bid{ OrderSide::BID, {}, 10.0, {} };
		const BotOrder ask{ OrderSide::ASK, {}, 14.0, {} };
		std::multiset<BotOrder> orders{ bid, ask, bid, ask };
		const BestOrder bestOrder{ 11.0, 15.0 };

		// Act
		const auto result = EraseFilledOrders(orders, bestOrder);

		// Assert
		ASSERT_EQ(result.size(), 2);
		ASSERT_EQ(result.begin()->Side, ask.Side);
		ASSERT_EQ(result.begin()->OrderId, ask.OrderId);
		ASSERT_EQ(result.begin()->Price, ask.Price);
		ASSERT_EQ(result.begin()->Volume, ask.Volume);

		ASSERT_EQ(orders.size(), 2);
		ASSERT_EQ(orders.begin()->Side, bid.Side);
		ASSERT_EQ(orders.begin()->OrderId, bid.OrderId);
		ASSERT_EQ(orders.begin()->Price, bid.Price);
		ASSERT_EQ(orders.begin()->Volume, bid.Volume);
	}

	TEST(UpdateWallet, AddsEthAndRemovesUsdForBidFilledOrders)
	{
		// Arrange
		const BotOrder bid{ OrderSide::BID, {}, 10.0, 1.2 };
		Wallet wallet{ 1.0, 15.0 };

		// Act
		UpdateWallet(wallet, { bid });

		// Assert
		ASSERT_DOUBLE_EQ(wallet.ETH, 2.2);
		ASSERT_DOUBLE_EQ(wallet.USD, 3);
	}

	TEST(UpdateWallet, RemovessEthAndAsssUsdForAskFilledOrders)
	{
		// Arrange
		const BotOrder ask{ OrderSide::ASK, {}, 10.0, 1.2 };
		Wallet wallet{ 1.5, 15.0 };

		// Act
		UpdateWallet(wallet, { ask });

		// Assert
		ASSERT_DOUBLE_EQ(wallet.ETH, 0.3);
		ASSERT_DOUBLE_EQ(wallet.USD, 27.0);
	}
}

