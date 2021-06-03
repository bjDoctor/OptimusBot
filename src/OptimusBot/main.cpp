// OptimusBot.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <memory>
#include "DvfSimulator.h"
#include "Utilities.h"

using namespace OptimusBot::Utilities;

/// @brief Active object responsible for periodically the simulated market and keeping track of the assets hold
class Bot final
{
public:
    Bot(std::unique_ptr<IDvfSimulator>&& simulator, double initialETH, double initialUSD) 
        : m_Simulator{std::move(simulator)}, m_Wallet{ initialETH , initialUSD }
    {
    }

    /// @brief Places initial, should be called before starting the Bot's "message loop"
    /// @return False if the best bid/ask pair cannot be retrieved. True otherwise
    bool PlaceInitialOrders(int numberOfOrdersEachSide)
    {
        //Initial order book
        const auto initialOrderBook = m_Simulator->GetOrderBook();

        //Initial best bid/ask pair
        auto initialBestOrder = ExtractBestOrder(initialOrderBook);
        if (!initialBestOrder)
        {
            std::cout << "Failed to retrieve initial best bid/ask pair. Terminating application." << std::endl;
            return false;
        }

        auto placeOrderDelegate = [&](double price, double amount) {
            return m_Simulator->PlaceOrder(price, amount);
        };
        m_PendingOrders = PlacePrudentOrders(m_Wallet, initialBestOrder.value(), numberOfOrdersEachSide, placeOrderDelegate);

        return true;
    }

    /// @brief Starts the trading session. Runs until all the pending orders are filled or an error occurs
    void StartTradingSession()
    {
        using namespace std::chrono_literals;
        constexpr auto marketRefreshInterval = 5s;
        constexpr auto assetBalancesInterval = 30s;

        auto now = std::chrono::system_clock::now();
        auto nextMarketRefresh = now + marketRefreshInterval;
        auto nextAssetBalances = now + assetBalancesInterval;

        //"message loop", refresh the market state every 5 seconds & prints assets every 30s
        while (!m_PendingOrders.empty())
        {
            now = std::chrono::system_clock::now();

            if (now > nextMarketRefresh)
            {
                nextMarketRefresh = now + marketRefreshInterval;

                auto orderBook = m_Simulator->GetOrderBook();
                auto bestOrder = ExtractBestOrder(orderBook);
                if (!bestOrder)
                {
                    std::cout << "Best bid/ask pair cannot be retrieved. Closing session." << std::endl;
                    return;
                }

                const auto filledOrders = EraseFilledOrders(m_PendingOrders, bestOrder.value());

                UpdateWallet(m_Wallet, filledOrders);

                if (now > nextAssetBalances)
                {
                    return;
                    nextAssetBalances = now + assetBalancesInterval;

                    std::cout << "\tWallet composed of " << m_Wallet.ETH << " ETH and " << m_Wallet.USD << " USD" << std::endl;
                    std::cout << "\tRemaining pending orders: " << std::endl;
                    for (const auto& order : m_PendingOrders)
                    {
                        std::cout << "\t\t" << " @ " << order.Price
                            << " : " << order.Volume
                            << " " << (order.Side == OrderSide::BID ? "BID" : "ASK")
                            << " (Id: " << order.OrderId << ")"
                            << std::endl;
                    }
                }
            }
        }

        if (m_PendingOrders.empty())
            std::cout << "All pending orders have been filled. Gracefuly losing trading session." << std::endl;
        else
        {
            std::cout << "Something went wrong... Cancelling remaining pending orders and closing trading session." << std::endl;
            for (const auto& order : m_PendingOrders)
                m_Simulator->CancelOrder(order.OrderId); //TODO: handle failure here?
        }
    }


private:
    // Simulator's lifetime is tied to the Bot
    std::unique_ptr<IDvfSimulator> m_Simulator;

    // Keeps track of the number of ETH and USD currently hold
    Wallet m_Wallet;

    // Orders still waiting to be filled
    std::multiset<BotOrder> m_PendingOrders;
};

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
        return false;
    }

    // Start trading!
    bot.StartTradingSession();

    return true;
}