#include "RiskManager.hpp"



RiskCheckResult RiskManager::checkOrder(const Order& order) const{
    if(order.side != Side::Buy && order.side != Side::Sell) {
        
    }
}