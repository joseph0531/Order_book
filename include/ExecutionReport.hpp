#pragma once

#include "OrderState.hpp"


enum class ExecutionEvent {
    Accepted,
    Rejected,
    Trade,
    Cancelled
};


struct ExecutionReport {
    int orderId;
    ExecutionEvent event;
    OrderStatus status;
    int lastFillPrice;
    int lastFillQuantity;
    int remainingQuantity;
};