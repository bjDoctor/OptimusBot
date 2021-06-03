#pragma once

#include <functional>
#include <algorithm>

#include "DvfSimulator.h"
#include "Types.h"

using namespace OptimusBot::Types;

/// @brief Grouping of static utilities, implemented as pure functions if possible, used throughout this app
namespace OptimusBot::Utilities
{
	/// @brief Random number generator within a given range
	/// @param min Lower bound
	/// @param max Upper bound
	/// @return A random number with a single decimal point
	double RandomWithSingleDecimalPoint(double min, double max) noexcept
	{
		if (min < 0 || max - min < 1)
			return 0.0;

		const auto rnd = min + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (max - min)));

		return static_cast<double>(static_cast<int>(rnd * 10.)) / 10.;
		//return static_cast<int>(rnd);
	}

	/// @brief Places bid/ask orders, by delegating the work to a lambda, using a "prudent" strategy, ensuring that we have enough assets to cover all orders
	/// @param wallet Assets currently hold
	/// @param bestOrder Current best bid/ask pair
	/// @param numberOfOrders Number of bid or ask orders. In total, twice that number can be created, one bid and one ask per iteration
	/// @param placeOrder Lambda, delegating the responsability of calling the relevent API to place the order
	/// @return A multiset of placed orders. If, for any reason, an order cannot be placed, it will not appear in the output.
	std::multiset<BotOrder> PlacePrudentOrders(const Wallet& wallet, const BestOrder& bestOrder, int numberOfOrders, const std::function<std::optional<IDvfSimulator::OrderID>(double, double)>& placeOrder) noexcept
	{
		if (numberOfOrders < 1)
			return {};

		std::multiset<BotOrder> orders;

		// This model the prudent approach, ensuring that the sum of all the orders
		// does not exceed the current assets hold
		const auto maxVolumePerOrder = wallet.ETH / numberOfOrders;

		for (int i = 0; i < numberOfOrders; i++)
		{
			{
				const auto bidPrice = RandomWithSingleDecimalPoint(0.95 * bestOrder.Bid, bestOrder.Bid);
				const auto bidVolume = RandomWithSingleDecimalPoint(0.1, maxVolumePerOrder);
				const auto bidOrderId = placeOrder(bidPrice, bidVolume);
				if (bidOrderId)
					orders.emplace( OrderSide::BID, bidOrderId.value(), bidPrice, bidVolume );
			}

			{
				const auto askPrice = RandomWithSingleDecimalPoint(bestOrder.Ask, 1.05 * bestOrder.Ask);
				const auto askVolume = RandomWithSingleDecimalPoint(0.1, maxVolumePerOrder);
				const auto askOrderId = placeOrder(askPrice, -askVolume);
				if (askOrderId)
					orders.emplace( OrderSide::ASK, askOrderId.value(), askPrice, askVolume );
			}
		}

		return orders;
	}

	/// @brief Extracts the best bid/ask pair from an order book
	/// @param orderBook Order book, as returned by IDvfSimulator::GetOrderBook
	/// @return An optional best bid/ask pair
	std::optional<BestOrder> ExtractBestOrder(const IDvfSimulator::OrderBook& orderBook) noexcept
	{
		auto sortedOrderBook = orderBook;
		std::sort(sortedOrderBook.begin(), sortedOrderBook.end());

		const auto size = sortedOrderBook.size();

		for (auto i = 0; i < size - 1; i++)
			if (sortedOrderBook[i].second > 0 && sortedOrderBook[i + 1].second < 0)
				return BestOrder{ sortedOrderBook[i].first, sortedOrderBook[i + 1].first };

		//failed to retrieve the best order
		return {};
	}

	/// @brief Erases the orders that have been filled. This is a non-pure function modifying the input orders
	/// @param orders Bot orders, passed by reference
	/// @param bestOrder Pair of current best bis/ask
	/// @return A multiset of the filled orders (i.e. the ones that have been removed from the input orders)
	std::multiset<BotOrder> EraseFilledOrders(std::multiset<BotOrder>& orders, const BestOrder& bestOrder) noexcept
	{
		std::multiset<BotOrder> filledOrders;
		const auto copiedOrders = orders; //copy input to allow erasing orders while iterating

		for (const auto& order : copiedOrders)
		{
			if (order.Side == OrderSide::BID && order.Price > bestOrder.Bid
				|| order.Side == OrderSide::ASK && order.Price < bestOrder.Ask)
			{
				filledOrders.insert(order);
				orders.erase(order);
			}
		}

		return filledOrders;
	}


	/// @brief Updates the wallet to reflect the changes of the filled orders on the assets hold. This is a non-pure function modifying the wallet object
	/// @param wallet Wallet to update
	/// @param filledOrders Filled orders to process
	void UpdateWallet(Wallet& wallet, std::multiset<BotOrder> filledOrders) noexcept
	{
		for (const auto& order : filledOrders)
		{
			if (order.Side == OrderSide::BID)
			{
				wallet.ETH += order.Volume;
				wallet.USD -= order.Volume * order.Price;
			}
			else if (order.Side == OrderSide::ASK)
			{
				wallet.ETH -= order.Volume;
				wallet.USD += order.Volume * order.Price;
			}
		}
	}
}
