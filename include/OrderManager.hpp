#pragma once


#include "Order.hpp"
#include "OrderState.hpp"
#include <unordered_map>


class OrderManager {
public:
    bool createOrder (const Order& order);

    const OrderState* getOrder (int orderId) const;

    bool hasOrder(int orderId) const;

    std::size_t orderCount() const;

private:

    std::unordered_map<int, OrderState> orders_;

};