// OptimusBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include "XXX.h"
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

    //Initial Wallet, wuth 10 ETH and 2000 USD
    Wallet initialWallet(10.0, 2000.0);

    //Initial order book
    const auto initialOrderBook = sim->GetOrderBook();    
    PrintOrders(initialOrderBook);

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
    auto placedOrders = PlacePrudentOrders(initialWallet, initialBestOrder.value(), 5, placeOrderDelegate);

    using namespace std::chrono_literals;
    auto now = std::chrono::system_clock::now();
    auto nextMarketRefresh = now + 5s;
    auto nextAssetBalances = now + 30s;
    //outer loop, refresh the market state every 5 seconds
    while (true) 
    {
        now = std::chrono::system_clock::now();

        if (now > nextMarketRefresh)
        {
            nextMarketRefresh = now + 5s;
            auto orderBook = sim->GetOrderBook();
            std::cout << "Refreshing order book at " << now.time_since_epoch().count() << std::endl;
            //PrintOrders(orderBook);
            auto bestOrder = ExtractBestOrder(orderBook).value();
            std::cout << "The order book's best bid/ask pair is: " << bestOrder.Bid << "/" << bestOrder.Ask << std::endl;
        }

        if (now > nextAssetBalances)
        {
            nextAssetBalances = now + 30s;
            std::cout << "Print asset balances..." << std::endl;
        }
    }

    //PrintOrders();


    return true;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
