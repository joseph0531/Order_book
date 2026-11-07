#include "TradingEngine.hpp"
#include <stdexcept>


SubmitResult TradingEngine::submitOrder(const Order& order) {
    SubmitResult result;
    result.accepted = false;
    result.trades = {};
    if(!orderManager_.createOrder(order)) {
        result.accepted = false;
        result.trades = {};
        return result;
    }
    
    if (!isValidOrder(order)) {
        orderManager_.rejectOrder(order.id);
        result.accepted = false;
        return result;
    }

    if(!orderManager_.acceptOrder(order.id)) {
        result.accepted = false;
        result.trades = {};
        return result;
    }

    result.trades = orderBook_.addOrder(order);
    result.accepted = true;
    
    for(const Trade& i : result.trades) {
        if(!orderManager_.applyFill(i.buyOrderId, i.quantity)) {
            throw std::logic_error("trading faild");
        };
        if(!orderManager_.applyFill(i.sellOrderId, i.quantity)) {
            throw std::logic_error("trading faild");

        };
    }
    return result;
}



const OrderState* TradingEngine::getOrderState(int orderId) const{
    return orderManager_.getOrder(orderId);
}


std::size_t TradingEngine::restingOrderCount() const {
    return orderBook_.orderCount();
}

bool TradingEngine::isValidOrder(const Order& order) const{
    if (order.side != Side::Buy && order.side != Side::Sell) {
        return false;
    }

    if (order.price <= 0) {
        return false;
    }

    if (order.quantity <= 0) {
        return false;
    }

    return true;
}