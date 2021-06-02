#pragma once

#include <optional>

namespace OptimusBot::Bot
{
	//Immutable object representing the ETH-USD assets currently hold
	struct Wallet
	{
		Wallet (double eth, double usd) : ETH{ eth }, USD{ usd }
		{}

		const double ETH;
		const double USD;
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
	struct Order
	{
		Order(OrderSide side, IDvfSimulator::OrderID orderId, double price, double volume)
			: Side{side}, OrderId{orderId}, Price{price}, Volume{volume}
		{}

		const OrderSide Side;
		const IDvfSimulator::OrderID OrderId;
		const double Price;
		const double Volume;
	};
}