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
    return it == orders_.end();
}

std::size_t OrderManager::orderCount() const{
    return orders_.size();

}