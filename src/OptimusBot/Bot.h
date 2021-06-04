#pragma once

#include <memory>
#include "DvfSimulator.h"
#include "Types.h"

namespace OptimusBot 
{
    /// @brief Active object responsible for periodically polling the simulated market and keeping track of the assets hold
    class Bot final
    {
    public:
        Bot(std::unique_ptr<IDvfSimulator>&& simulator, double initialETH, double initialUSD)
            : m_Simulator{ std::move(simulator) }, m_Wallet{ initialETH , initialUSD }
        {
        }

        /// @brief Places initial, should be called before starting the Bot's "message loop"
        /// @return False if the best bid/ask pair cannot be retrieved. True otherwise
        bool PlaceInitialOrders(int numberOfOrdersEachSide);

        /// @brief Starts the trading session. Runs until all the pending orders are filled or an error occurs
        void StartTradingSession();


    private:
        // Simulator's lifetime is tied to the Bot
        std::unique_ptr<IDvfSimulator> m_Simulator;

        // Keeps track of the number of ETH and USD currently hold
        Types::Wallet m_Wallet;

        // Orders still waiting to be filled
        std::multiset<Types::BotOrder> m_PendingOrders;
    };

}


