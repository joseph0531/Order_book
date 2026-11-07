#pragma once
#include "Order.hpp"




enum class OrderStatus {
    PendingNew,
    Accepted,
    PartiallyFilled,
    Filled,
    Cancelled,
    Rejected
};


struct OrderState {
    int id;
    Side side;
    int price;
    int originalQuantity;
    int remainingQuantity;
    OrderStatus status;
};