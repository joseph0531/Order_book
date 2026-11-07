#include "OrderManager.hpp"




bool OrderManager::createOrder(const Order& order) {

    if (hasOrder(order.id)) {
        return false;
    }
    OrderState order_state;
    
    order_state.id = order.id;
    order_state.side = order.side;
    order_state.price = order.price;
    order_state.originalQuantity = order.quantity;
    order_state.remainingQuantity = order.quantity;
    order_state.status = OrderStatus::PendingNew;


    orders_.insert({order.id, order_state});

    return true;
}


const OrderState* OrderManager::getOrder (int orderId) const{
    auto it = orders_.find(orderId);
    if (it == orders_.end()) {
        return nullptr;
    }
    return &it->second;
}

bool OrderManager::hasOrder(int orderId) const{
    auto it = orders_.find(orderId);
    return it != orders_.end();
}

std::size_t OrderManager::orderCount() const{
    return orders_.size();

}


bool OrderManager::acceptOrder(int orderId) {

    auto It = orders_.find(orderId);

    if (It == orders_.end()) {
        return false;
    }
    auto state = &It->second;

    if (state->status != OrderStatus::PendingNew) {
        return false;
    } else {
        state->status = OrderStatus::Accepted;
        return true;
    }
}

bool OrderManager::rejectOrder(int orderId) {

    auto It = orders_.find(orderId);
    if (It == orders_.end()) {
        return false;
    }

    auto state= &It->second;

    if (state->status != OrderStatus::PendingNew) {
        return false;
    } else {
        state->status = OrderStatus::Rejected;
        state->remainingQuantity = state->originalQuantity;
        return true;
    }
}


bool OrderManager::applyFill(int orderId, int fillQuantity) {
    auto It = orders_.find(orderId);

    if (It == orders_.end()) {
        return false;
    }

    auto state = &It->second;

    if (state->status != OrderStatus::Accepted && state->status != OrderStatus::PartiallyFilled) {
        return false;
    }

    if (fillQuantity <= 0) {
        return false;
    }
    if (fillQuantity > state->remainingQuantity) {
        return false;
    }

    state->remainingQuantity -= fillQuantity;

    if (state->remainingQuantity == 0) {
        state->status = OrderStatus::Filled;
    } else {
        state->status = OrderStatus::PartiallyFilled;
    }

    return true;
}


bool OrderManager::cancelOrder(int orderId) {
    auto It = orders_.find(orderId);

    if (It == orders_.end()){
        return false;
    }

    auto state = &It->second;

    if (state->status != OrderStatus::Accepted && state->status != OrderStatus::PartiallyFilled) {
        return false;
    }

    state->status = OrderStatus::Cancelled;
    return true;
}