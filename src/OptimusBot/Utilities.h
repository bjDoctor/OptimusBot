#pragma once

#include <functional>

#include "XXX.h"
#include "Bot.h"

using namespace OptimusBot::Bot;

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
	}

	/// @brief Places bid/ask orders, by delegating the work to a lambda, using a "prudent" strategy, ensuring that we have enough assets to cover all orders
	/// @param wallet Assets currently hold
	/// @param bestOrder Current best bid/ask pair
	/// @param numberOfOrders Number of bid or ask orders. In total, twice that number can be created, one bid and one ask per iteration
	/// @param placeOrder Lambda, delegating the responsability of calling the relevent API to place the order
	/// @return A vector of placed orders. If, for any reason, an order cannot be placed, it will not appear in the output.
	std::vector<Order> PlacePrudentOrders(const Wallet& wallet, const BestOrder& bestOrder, int numberOfOrders, const std::function<std::optional<IDvfSimulator::OrderID>(double, double)>& placeOrder)
	{
		if (numberOfOrders < 1)
			return {};

		std::vector<Order> orders;

		// This model the prudent approach, ensureing that the sum of all the orders
		// does not exceed the current assets hold
		const auto maxVolumePerOrder = wallet.ETH / numberOfOrders;

		for (int i = 0; i < numberOfOrders; i++)
		{
			{
				const auto bidPrice = RandomWithSingleDecimalPoint(0.95 * bestOrder.Bid, bestOrder.Bid);
				const auto bidVolume = RandomWithSingleDecimalPoint(0.1, maxVolumePerOrder);
				const auto bidOrderId = placeOrder(bidPrice, bidVolume);
				if (bidOrderId)
					orders.push_back(Order{ OrderSide::BID, bidOrderId.value(), bidPrice, bidVolume });
			}

			{
				const auto askPrice = RandomWithSingleDecimalPoint(bestOrder.Ask, 1.05 * bestOrder.Ask);
				const auto askVolume = RandomWithSingleDecimalPoint(0.1, maxVolumePerOrder);
				const auto askOrderId = placeOrder(askPrice, -askVolume);
				if (askOrderId)
					orders.push_back(Order{ OrderSide::ASK, askOrderId.value(), askPrice, askVolume });
			}
		}

		return orders;
	}
}
