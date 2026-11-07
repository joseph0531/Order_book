#pragma once 

#include <vector>
#include "Order.hpp"
#include "Trade.hpp"
#include <cstddef>
#include <map>
#include <deque>
#include <functional>
#include <unordered_map>


class OrderBook {
//外部程式可以使用的功能
public:

    std::vector<Trade> addOrder(const Order& order);

    bool cancelOrder(int id);

    size_t orderCount() const;

    void printOrder() const;

    
    
    
    //只有內部可以使用操作的資料
private:
    
    struct OrderLocation {
        Side side;
        int price; 
    };
    
    bool canMatch(const Order& buy, const Order& sell) const;

    std::unordered_map<int, OrderLocation> orderLocations_;

    static const char* sideToString(Side side);

    Trade executeTrade(Order& buy, Order& sell, int tradePrice);

    bool isValidOrder(const Order& order) const;
    bool hasOrderId(int id) const;

    using PriceLevel = std::deque<Order>;

    std::map<int, PriceLevel, std::greater<int>> buyLevels_;
    std::map<int, PriceLevel> sellLevels_;

    void addBuyToLevel (const Order& order);
    void addSellToLevel (const Order& order);

    Order* getBestBuyOrder();
    Order* getBestSellOrder();

    void removeBestBuyOrder();
    void removeBestSellOrder();

    void processBuyOrder(Order& buy, std::vector<Trade>& trade);
    void processSellOrder(Order& sell, std::vector<Trade>& trade);
};