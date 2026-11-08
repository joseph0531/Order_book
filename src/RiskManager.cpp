#include "RiskManager.hpp"
#include <stdexcept>

RiskManager::RiskManager(const RiskLimits& limits) : limits_(limits){

    if (limits_.maxOrderQuantity <= 0) {
        throw std::invalid_argument("maxOrderQuantity must be greater than 0");
    }
}



RiskCheckResult RiskManager::checkOrder(const Order& order) const{
    if(order.side != Side::Buy && order.side != Side::Sell) {
        return {false, RiskRejectReason::InvalidSide};
    }
    if(order.price <= 0) {
        return {false, RiskRejectReason::InvalidPrice};
    }
    if(order.quantity <= 0) {
        return {false, RiskRejectReason::InvalidQuantity};
    }
    if(order.quantity > limits_.maxOrderQuantity) {
        return {false, RiskRejectReason::OrderQuantityLimitExceeded};
    }

    return {true, RiskRejectReason::None};
}