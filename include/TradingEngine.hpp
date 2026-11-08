#pragma once
#include "Trade.hpp"
#include <vector>
#include "OrderManager.hpp"
#include "OrderBook.hpp"
#include "ExecutionReport.hpp"
#include "RiskManager.hpp"


struct SubmitResult{
    bool accepted;
    std::vector<Trade> trades;
    std::vector<ExecutionReport> reports;
};

struct CancelResult{
    bool cancelled;
    std::vector<ExecutionReport> reports;
};



class TradingEngine {
public:
    SubmitResult submitOrder(const Order& order);

    const OrderState* getOrderState(int orderId) const;
    
    std::size_t restingOrderCount() const;

    CancelResult cancelOrder(int orderId);

    explicit TradingEngine(const RiskLimits& limits);


private:

    OrderManager orderManager_;
    OrderBook orderBook_;
    RiskManager riskManager_;
};