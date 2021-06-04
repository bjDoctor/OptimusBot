// OptimusBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include "Bot.h"

using namespace OptimusBot;

int main()
{
    // Initial assets
    constexpr auto initialETH = 10.0;
    constexpr auto initialUSD = 2000.0;

    // Make the bot
    Bot bot{ std::unique_ptr<IDvfSimulator>(DvfSimulator::Create()), initialETH,  initialUSD };

    // Place 5 bid and 5 ask initial orders
    const auto initialOrderPlaced = bot.PlaceInitialOrders(5);
    if (!initialOrderPlaced)
    {
        std::cout << "Failed to place inital orders, closing the application..." << std::endl;
        return 0;
    }

    // Start trading!
    bot.StartTradingSession();

    return 1;
}