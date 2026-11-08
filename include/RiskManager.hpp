
#pragma once

#include "Order.hpp"

enum class RiskRejectReason {
    None,
    InvalidSide,
    InvalidPrice,
    InvalidQuantity,
    OrderQuantityLimitExceeded
};


struct RiskCheckResult {
    bool approved;
    RiskRejectReason reason;
};


struct RiskLimits {
    int maxOrderQuantity;
};

class RiskManager {
public:
    explicit RiskManager(const RiskLimits& limits);

    RiskCheckResult checkOrder(const Order& order) const;

private:
    RiskLimits limits_;
};