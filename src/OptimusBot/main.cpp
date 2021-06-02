// OptimusBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "XXX.h"
#include "Utilities.h"

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
    auto* sim = DvfSimulator::Create();



    //Initial order book
    PrintOrders(sim->GetOrderBook());

    //Initial Wallet
    Wallet initialWallet(10.0, 2000.0);

    //Initial best bid/ask pair
    BestOrder initialBestOrder(175.0, 225.0);
    

    auto placeOrder = [&sim](double price, double amount) {
        return sim->PlaceOrder(price, amount);
    };
    auto placedOrders = OptimusBot::Utilities::PlacePrudentOrders(initialWallet, initialBestOrder, 5, placeOrder);

    

    PrintOrders(sim->GetOrderBook());

    //std::cout << OptimusBot::Utilities::RandomWithSingleDecimalPoint(0.1, 96.5);

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
