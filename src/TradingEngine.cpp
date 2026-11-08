#include "TradingEngine.hpp"
#include <stdexcept>


SubmitResult TradingEngine::submitOrder(const Order& order) {
    SubmitResult result;
    result.accepted = false;
    result.trades = {};
    if(!orderManager_.createOrder(order)) {
        result.accepted = false;
        result.trades = {};
        result.reports = {ExecutionReport{order.id, ExecutionEvent::Rejected, OrderStatus::Rejected, 0,0, order.quantity}};
        return result;
    }
    
    if (!isValidOrder(order)) {
        orderManager_.rejectOrder(order.id);
        result.reports = {ExecutionReport{order.id, ExecutionEvent::Rejected, OrderStatus::Rejected, 0,0, order.quantity}};
        result.accepted = false;
        return result;
    }

    if(!orderManager_.acceptOrder(order.id)) {
        result.accepted = false;
        result.trades = {};
        return result;
    }
    result.reports = {
        ExecutionReport{order.id, ExecutionEvent::Accepted, OrderStatus::Accepted, 0, 0, order.quantity}
    };

    result.trades = orderBook_.addOrder(order);
    result.accepted = true;

    
    for(const Trade& i : result.trades) {
        if(!orderManager_.applyFill(i.buyOrderId, i.quantity)) {
            throw std::logic_error("trading failed");
        }
        result.reports.push_back(ExecutionReport{i.buyOrderId, ExecutionEvent::Trade, orderManager_.getOrder(i.buyOrderId)->status, i.price, i.quantity, orderManager_.getOrder(i.buyOrderId)->remainingQuantity});
        if(!orderManager_.applyFill(i.sellOrderId, i.quantity)) {
            throw std::logic_error("trading failed");
        }
        result.reports.push_back(ExecutionReport{i.sellOrderId, ExecutionEvent::Trade, orderManager_.getOrder(i.sellOrderId)->status, i.price, i.quantity, orderManager_.getOrder(i.sellOrderId)->remainingQuantity});
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

CancelResult TradingEngine::cancelOrder(int orderId) {
    CancelResult result;
    result.cancelled = false;
    result.reports = {};
    auto it = orderManager_.getOrder(orderId);

    if (it == nullptr) {
        return result;
    }
    if (it->status != OrderStatus::Accepted && it->status != OrderStatus::PartiallyFilled) {
        return result;
    }

    if(!orderBook_.cancelOrder(orderId)) {
        throw std::logic_error("cancel order failed");
    }

    if(!orderManager_.cancelOrder(orderId)) {
        throw std::logic_error("cancel order failed");
    }
    result.cancelled = true;
    result.reports = {ExecutionReport{
        orderId,
        ExecutionEvent::Cancelled,
        OrderStatus::Cancelled,
        0,
        0,
        orderManager_.getOrder(orderId)->remainingQuantity

    }
    };

    return result;
}