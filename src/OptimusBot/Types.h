#pragma once

#include <optional>

/// @brief Grouping of type definitions for small/simple objects used throughout this application
namespace OptimusBot::Types
{
	//Mutable object representing the ETH-USD currently hold
	struct Wallet
	{
		Wallet() = default;

		Wallet (double eth, double usd) : ETH{ eth }, USD{ usd }
		{}

		double ETH;
		double USD;
	};

	//Immutable object representing the current best bid/ask pair of the order book
	struct BestOrder
	{
		BestOrder(double bid, double ask) : Bid{ bid }, Ask{ ask }
		{}

		const double Bid;
		const double Ask;
	};

	enum class OrderSide 
	{
		BID,
		ASK
	};

	//Immutable object representing an order placed by the bot
	struct BotOrder
	{
		BotOrder(OrderSide side, IDvfSimulator::OrderID orderId, double price, double volume)
			: Side{side}, OrderId{orderId}, Price{price}, Volume{volume}
		{}

		const OrderSide Side;
		const IDvfSimulator::OrderID OrderId;
		const double Price;
		const double Volume;

		//Operator required to ensure BotOrders  can be sorted in a set/map
		bool operator < (const BotOrder& other) const noexcept
		{
			return Price < other.Price;
		}
	};
}