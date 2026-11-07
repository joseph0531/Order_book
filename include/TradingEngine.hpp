#pragma once
#include "Trade.hpp"
#include <vector>
#include "OrderManager.hpp"
#include "OrderBook.hpp"


struct SubmitResult{
    bool accepted;
    std::vector<Trade> trades;
};



class TradingEngine {
public:
    SubmitResult submitOrder(const Order& order);

    const OrderState* getOrderState(int orderId) const;
    
    std::size_t restingOrderCount() const;

private:

    bool isValidOrder(const Order& order) const;

    OrderManager orderManager_;
    OrderBook orderBook_;
};