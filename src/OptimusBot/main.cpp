// OptimusBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include "DvfSimulator.h"
#include "Utilities.h"

using namespace OptimusBot::Utilities;

template <class T>
void PrintOrders(const T& ob)
{
    std::cout << std::endl;
    std::cout << "Order book composed of:" << std::endl;
    for (const auto& order : ob)
    {
        std::cout << "\t" << " @ " << order.first
            << " : " << order.second
            << " " << (order.second > 0.0 ? "BID" : "ASK")
            << std::endl;
    }
    std::cout << "\n=====\n" << std::endl;
}

int main()
{
    // Creates the simulator
    auto* sim = DvfSimulator::Create();

    //Initial Wallet, with 10 ETH and 2000 USD
    Wallet wallet(10.0, 2000.0);

    //Initial order book
    const auto initialOrderBook = sim->GetOrderBook();    

    //Initial best bid/ask pair
    auto initialBestOrder = ExtractBestOrder(initialOrderBook);
    if (!initialBestOrder)
    {
        std::cout << "ERROR: Failed to retrieve initial best bid/ask pair. Terminating application." << std::endl;
        return false;
    }
    std::cout << "The order book's best bid/ask pair is: " << initialBestOrder.value().Bid << "/" << initialBestOrder.value().Ask << std::endl;

    auto placeOrderDelegate = [&sim](double price, double amount) {
        return sim->PlaceOrder(price, amount);
    };
    auto pendingOrders = PlacePrudentOrders(wallet, initialBestOrder.value(), 5, placeOrderDelegate);

    using namespace std::chrono_literals;
    auto now = std::chrono::system_clock::now();
    auto nextMarketRefresh = now + 5s;
    auto nextAssetBalances = now + 30s;

    //outer loop, refresh the market state every 5 seconds & print asserts every 30s
    while (!pendingOrders.empty()) 
    {
        now = std::chrono::system_clock::now();

        if (now > nextMarketRefresh)
        {
            nextMarketRefresh = now + 5s;
            auto orderBook = sim->GetOrderBook();
            //std::cout << "Refreshing order book at " << now.time_since_epoch().count() << std::endl;
            //PrintOrders(orderBook);
            auto bestOrder = ExtractBestOrder(orderBook).value();
            //std::cout << "The order book's best bid/ask pair is: " << bestOrder.Bid << "/" << bestOrder.Ask << std::endl;

            const auto filledOrders = EraseFilledOrders(pendingOrders, bestOrder);

            UpdateWallet(wallet, filledOrders);

            if (now > nextAssetBalances)
            {
                nextAssetBalances = now + 30s;
                //std::cout << "Print asset balances..." << std::endl;
                //PrintOrders(placedOrders);
                std::cout << "Wallet composed of " << wallet.ETH << " ETH and " << wallet.USD << " USD" << std::endl;
                std::cout << "Remaining pending orders: " << std::endl;
                for (const auto& order : pendingOrders)
                {
                    std::cout << "\t" << " @ " << order.Price
                        << " : " << order.Volume
                        << " " << (order.Side == OrderSide::BID ? "BID" : "ASK")
                        << " (Id: " << order.OrderId << ")"
                        << std::endl;
                }
            }
        }

        
    }

    if (pendingOrders.empty())
        std::cout << "All pending orders have been filled. Gracefuly losing trading session." << std::endl;
    else
    {
        std::cout << "Something went wrong... Cancelling remaining pending orders and closing trading session." << std::endl;
        for (const auto& order : pendingOrders)
            sim->CancelOrder(order.OrderId); //TODO: handle failure here?
    }


    return true;
}