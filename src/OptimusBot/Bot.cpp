#include "pch.h"
#include "Bot.h"
#include "Utilities.h"

using namespace OptimusBot::Utilities;
using namespace OptimusBot::Types;


namespace 
{
    void PrintAssets(const Wallet& wallet, const std::multiset<BotOrder>& pendingOrders)
    {
        std::cout << "\tWallet composed of " << wallet.ETH << " ETH and " << wallet.USD << " USD" << std::endl;

        if (!pendingOrders.empty())
        {
            std::cout << "\tRemaining pending orders: " << std::endl;
            for (const auto& order : pendingOrders)
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

bool OptimusBot::Bot::PlaceInitialOrders(int numberOfOrdersEachSide)
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


void OptimusBot::Bot::StartTradingSession()
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
                nextAssetBalances = now + assetBalancesInterval;
                PrintAssets(m_Wallet, m_PendingOrders);
            }
        }
    }

    PrintAssets(m_Wallet, m_PendingOrders);

    if (m_PendingOrders.empty())
        std::cout << "All pending orders have been filled. Gracefuly losing trading session." << std::endl;
    else
    {
        std::cout << "Something went wrong... Cancelling remaining pending orders and closing trading session." << std::endl;
        for (const auto& order : m_PendingOrders)
            m_Simulator->CancelOrder(order.OrderId); //TODO: handle failure here?
    }
}
