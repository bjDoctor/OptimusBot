#pragma once

#include <functional>
#include "DvfSimulator.h"
#include "Types.h"

/// @brief Grouping of static utilities, implemented as pure functions if possible, used throughout this app
namespace OptimusBot::Utilities
{
	/// @brief Random number generator within a given range (but always greater than zero)
	/// @param min Lower bound
	/// @param max Upper bound
	/// @return A positive random number with a single decimal point
	double Random(double min, double max) noexcept;

	/// @brief Places bid/ask orders, by delegating the work to a lambda, using a "prudent" strategy, ensuring that we have enough assets to cover all orders
	/// @param wallet Assets currently hold
	/// @param bestOrder Current best bid/ask pair
	/// @param numberOfOrders Number of bid or ask orders. In total, twice that number can be created, one bid and one ask per iteration
	/// @param placeOrder Lambda, delegating the responsability of calling the relevent API to place the order
	/// @return A multiset of placed orders. If, for any reason, an order cannot be placed, it will not appear in the output.
	std::multiset<Types::BotOrder> PlacePrudentOrders(const Types::Wallet& wallet, const Types::BestOrder& bestOrder, int numberOfOrders, const std::function<std::optional<IDvfSimulator::OrderID>(double, double)>& placeOrder) noexcept;

	/// @brief Extracts the best bid/ask pair from an order book
	/// @param orderBook Order book, as returned by the market simulator
	/// @return An optional best bid/ask pair
	std::optional<Types::BestOrder> ExtractBestOrder(const IDvfSimulator::OrderBook& orderBook) noexcept;

	/// @brief Erases the orders that have been filled. This is a non-pure function modifying the input orders
	/// @param orders Bot orders, passed by reference
	/// @param bestOrder Pair of current best bis/ask
	/// @return A multiset of the filled orders (i.e. the ones that have been removed from the input orders)
	std::multiset<Types::BotOrder> EraseFilledOrders(std::multiset<Types::BotOrder>& orders, const Types::BestOrder& bestOrder) noexcept;


	/// @brief Updates the wallet to reflect the changes of the filled orders on the assets hold. This is a non-pure function modifying the wallet input
	/// @param wallet Wallet to update
	/// @param filledOrders Filled orders to process
	void UpdateWallet(Types::Wallet& wallet, std::multiset<Types::BotOrder> filledOrders) noexcept;
}
