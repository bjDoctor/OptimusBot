#include "pch.h"
#include "Utilities.h"

using namespace OptimusBot::Types;

double OptimusBot::Utilities::RandomWithSingleDecimalPoint(double min, double max) noexcept
{
	if (min < 0 || max - min < 1)
		return 0.0;

	const auto rnd = min + static_cast <double> (rand()) / (static_cast <double> (RAND_MAX / (max - min)));

	return static_cast<double>(static_cast<int>(rnd * 10.)) / 10.;
}


std::multiset<BotOrder> OptimusBot::Utilities::PlacePrudentOrders(const Wallet& wallet, const BestOrder& bestOrder, int numberOfOrders, const std::function<std::optional<IDvfSimulator::OrderID>(double, double)>& placeOrder) noexcept
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
				orders.emplace(OrderSide::BID, bidOrderId.value(), bidPrice, bidVolume);
		}

		{
			const auto askPrice = RandomWithSingleDecimalPoint(bestOrder.Ask, 1.05 * bestOrder.Ask);
			const auto askVolume = RandomWithSingleDecimalPoint(0.1, maxVolumePerOrder);
			const auto askOrderId = placeOrder(askPrice, -askVolume);
			if (askOrderId)
				orders.emplace(OrderSide::ASK, askOrderId.value(), askPrice, askVolume);
		}
	}

	return orders;
}


std::optional<BestOrder> OptimusBot::Utilities::ExtractBestOrder(const IDvfSimulator::OrderBook& orderBook) noexcept
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


std::multiset<BotOrder> OptimusBot::Utilities::EraseFilledOrders(std::multiset<BotOrder>& orders, const BestOrder& bestOrder) noexcept
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


void OptimusBot::Utilities::UpdateWallet(Wallet& wallet, std::multiset<BotOrder> filledOrders) noexcept
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
