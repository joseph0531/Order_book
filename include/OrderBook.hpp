#pragma once 

#include <vector>
#include "Order.hpp"
#include "Trade.hpp"
#include <cstddef>
#include <unordered_set>
#include <map>
#include <deque>
#include <functional>


class OrderBook {
//外部程式可以使用的功能
public:

    std::vector<Trade> addOrder(const Order& order);

    bool cancelOrder(std::string side, int id);

    void printOrder() const;

    size_t orderCount() const;

    bool canMatch(const Order& buy, const Order& sell) const;



//只有內部可以使用操作的資料
private:

    std::vector<Order> buyOrders_;
    std::vector<Order> sellOrders_;
    std::unordered_set<int> orderids_;

    int findBestBuyIndex() const;
    int findBestSellIndex() const;

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

};