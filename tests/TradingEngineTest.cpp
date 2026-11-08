#include <cassert>

#include "TradingEngine.hpp"

void testRestingOrderAndFullMatch() {
    TradingEngine engine{RiskLimits{100}};

    const SubmitResult sellResult =
        engine.submitOrder({1, Side::Sell, 100, 10});

    assert(sellResult.accepted);
    assert(sellResult.trades.empty());
    assert(sellResult.reports.size() == 1);
    assert(sellResult.reports[0].orderId == 1);
    assert(sellResult.reports[0].event == ExecutionEvent::Accepted);
    assert(sellResult.reports[0].status == OrderStatus::Accepted);
    assert(sellResult.reports[0].lastFillPrice == 0);
    assert(sellResult.reports[0].lastFillQuantity == 0);
    assert(sellResult.reports[0].remainingQuantity == 10);
    assert(engine.restingOrderCount() == 1);

    const OrderState* sellState = engine.getOrderState(1);
    assert(sellState != nullptr);
    assert(sellState->status == OrderStatus::Accepted);
    assert(sellState->remainingQuantity == 10);

    const SubmitResult buyResult =
        engine.submitOrder({2, Side::Buy, 100, 10});

    assert(buyResult.accepted);
    assert(buyResult.trades.size() == 1);
    assert(buyResult.reports.size() == 3);
    assert(buyResult.reports[0].orderId == 2);
    assert(buyResult.reports[0].event == ExecutionEvent::Accepted);
    assert(buyResult.reports[1].orderId == 2);
    assert(buyResult.reports[1].event == ExecutionEvent::Trade);
    assert(buyResult.reports[1].status == OrderStatus::Filled);
    assert(buyResult.reports[1].lastFillPrice == 100);
    assert(buyResult.reports[1].lastFillQuantity == 10);
    assert(buyResult.reports[1].remainingQuantity == 0);
    assert(buyResult.reports[2].orderId == 1);
    assert(buyResult.reports[2].event == ExecutionEvent::Trade);
    assert(buyResult.reports[2].status == OrderStatus::Filled);
    assert(buyResult.reports[2].lastFillPrice == 100);
    assert(buyResult.reports[2].lastFillQuantity == 10);
    assert(buyResult.reports[2].remainingQuantity == 0);

    const Trade& trade = buyResult.trades.front();
    assert(trade.buyOrderId == 2);
    assert(trade.sellOrderId == 1);
    assert(trade.price == 100);
    assert(trade.quantity == 10);

    const OrderState* filledSellState = engine.getOrderState(1);
    const OrderState* filledBuyState = engine.getOrderState(2);
    assert(filledSellState != nullptr);
    assert(filledBuyState != nullptr);
    assert(filledSellState->status == OrderStatus::Filled);
    assert(filledBuyState->status == OrderStatus::Filled);
    assert(filledSellState->remainingQuantity == 0);
    assert(filledBuyState->remainingQuantity == 0);
    assert(engine.restingOrderCount() == 0);
}

void testInvalidOrderIsRejected() {
    TradingEngine engine{RiskLimits{100}};

    const SubmitResult result =
        engine.submitOrder({3, Side::Buy, -1, 10});

    assert(!result.accepted);
    assert(result.trades.empty());
    assert(result.reports.size() == 1);
    assert(result.reports[0].orderId == 3);
    assert(result.reports[0].event == ExecutionEvent::Rejected);
    assert(result.reports[0].status == OrderStatus::Rejected);
    assert(result.reports[0].lastFillPrice == 0);
    assert(result.reports[0].lastFillQuantity == 0);
    assert(result.reports[0].remainingQuantity == 10);

    const OrderState* state = engine.getOrderState(3);
    assert(state != nullptr);
    assert(state->status == OrderStatus::Rejected);
    assert(state->remainingQuantity == 10);
    assert(engine.restingOrderCount() == 0);
}

void testDuplicateHistoricalIdIsRejectedWithoutOverwrite() {
    TradingEngine engine{RiskLimits{100}};

    assert(engine.submitOrder({4, Side::Sell, 100, 5}).accepted);
    assert(engine.submitOrder({5, Side::Buy, 100, 5}).accepted);

    const OrderState* originalState = engine.getOrderState(4);
    assert(originalState != nullptr);
    assert(originalState->status == OrderStatus::Filled);

    const SubmitResult duplicateResult =
        engine.submitOrder({4, Side::Buy, 200, 20});

    assert(!duplicateResult.accepted);
    assert(duplicateResult.trades.empty());
    assert(duplicateResult.reports.size() == 1);
    assert(duplicateResult.reports[0].orderId == 4);
    assert(duplicateResult.reports[0].event == ExecutionEvent::Rejected);
    assert(duplicateResult.reports[0].status == OrderStatus::Rejected);
    assert(duplicateResult.reports[0].lastFillPrice == 0);
    assert(duplicateResult.reports[0].lastFillQuantity == 0);
    assert(duplicateResult.reports[0].remainingQuantity == 20);

    const OrderState* stateAfterDuplicate = engine.getOrderState(4);
    assert(stateAfterDuplicate != nullptr);
    assert(stateAfterDuplicate->side == Side::Sell);
    assert(stateAfterDuplicate->price == 100);
    assert(stateAfterDuplicate->originalQuantity == 5);
    assert(stateAfterDuplicate->remainingQuantity == 0);
    assert(stateAfterDuplicate->status == OrderStatus::Filled);
    assert(engine.restingOrderCount() == 0);
}

void testCancelRestingOrder() {
    TradingEngine engine{RiskLimits{100}};

    assert(engine.submitOrder({10, Side::Buy, 100, 20}).accepted);
    assert(engine.restingOrderCount() == 1);
    const CancelResult cancelResult = engine.cancelOrder(10);
    assert(cancelResult.cancelled);
    assert(cancelResult.reports.size() == 1);
    assert(cancelResult.reports[0].orderId == 10);
    assert(cancelResult.reports[0].event == ExecutionEvent::Cancelled);
    assert(cancelResult.reports[0].status == OrderStatus::Cancelled);
    assert(cancelResult.reports[0].lastFillPrice == 0);
    assert(cancelResult.reports[0].lastFillQuantity == 0);
    assert(cancelResult.reports[0].remainingQuantity == 20);

    const OrderState* state = engine.getOrderState(10);
    assert(state != nullptr);
    assert(state->status == OrderStatus::Cancelled);
    assert(state->remainingQuantity == 20);
    assert(engine.restingOrderCount() == 0);
    const CancelResult secondCancelResult = engine.cancelOrder(10);
    assert(!secondCancelResult.cancelled);
    assert(secondCancelResult.reports.empty());
}

void testCancelPartiallyFilledOrder() {
    TradingEngine engine{RiskLimits{100}};

    assert(engine.submitOrder({20, Side::Sell, 100, 10}).accepted);
    const SubmitResult buyResult =
        engine.submitOrder({21, Side::Buy, 100, 4});

    assert(buyResult.accepted);
    assert(buyResult.trades.size() == 1);

    const OrderState* stateBeforeCancel = engine.getOrderState(20);
    assert(stateBeforeCancel != nullptr);
    assert(stateBeforeCancel->status == OrderStatus::PartiallyFilled);
    assert(stateBeforeCancel->remainingQuantity == 6);
    assert(engine.restingOrderCount() == 1);

    const CancelResult cancelResult = engine.cancelOrder(20);
    assert(cancelResult.cancelled);
    assert(cancelResult.reports.size() == 1);
    assert(cancelResult.reports[0].orderId == 20);
    assert(cancelResult.reports[0].event == ExecutionEvent::Cancelled);
    assert(cancelResult.reports[0].status == OrderStatus::Cancelled);
    assert(cancelResult.reports[0].lastFillPrice == 0);
    assert(cancelResult.reports[0].lastFillQuantity == 0);
    assert(cancelResult.reports[0].remainingQuantity == 6);

    const OrderState* stateAfterCancel = engine.getOrderState(20);
    assert(stateAfterCancel != nullptr);
    assert(stateAfterCancel->status == OrderStatus::Cancelled);
    assert(stateAfterCancel->remainingQuantity == 6);
    assert(engine.restingOrderCount() == 0);
}

void testCannotCancelTerminalOrUnknownOrder() {
    TradingEngine engine{RiskLimits{100}};

    assert(engine.submitOrder({30, Side::Sell, 100, 5}).accepted);
    assert(engine.submitOrder({31, Side::Buy, 100, 5}).accepted);
    assert(engine.getOrderState(30)->status == OrderStatus::Filled);
    const CancelResult filledCancelResult = engine.cancelOrder(30);
    assert(!filledCancelResult.cancelled);
    assert(filledCancelResult.reports.empty());

    const SubmitResult rejectedResult =
        engine.submitOrder({32, Side::Buy, 0, 5});
    assert(!rejectedResult.accepted);
    assert(engine.getOrderState(32)->status == OrderStatus::Rejected);
    const CancelResult rejectedCancelResult = engine.cancelOrder(32);
    assert(!rejectedCancelResult.cancelled);
    assert(rejectedCancelResult.reports.empty());

    const CancelResult unknownCancelResult = engine.cancelOrder(999);
    assert(!unknownCancelResult.cancelled);
    assert(unknownCancelResult.reports.empty());
}

void testRiskQuantityLimitIntegration() {
    TradingEngine engine{RiskLimits{10}};

    const SubmitResult boundaryResult =
        engine.submitOrder({40, Side::Buy, 100, 10});
    assert(boundaryResult.accepted);
    assert(engine.restingOrderCount() == 1);

    const SubmitResult overLimitResult =
        engine.submitOrder({41, Side::Buy, 100, 11});
    assert(!overLimitResult.accepted);
    assert(overLimitResult.trades.empty());
    assert(overLimitResult.reports.size() == 1);
    assert(overLimitResult.reports[0].event == ExecutionEvent::Rejected);

    const OrderState* rejectedState = engine.getOrderState(41);
    assert(rejectedState != nullptr);
    assert(rejectedState->status == OrderStatus::Rejected);
    assert(engine.restingOrderCount() == 1);
}

int main() {
    testRestingOrderAndFullMatch();
    testInvalidOrderIsRejected();
    testDuplicateHistoricalIdIsRejectedWithoutOverwrite();
    testCancelRestingOrder();
    testCancelPartiallyFilledOrder();
    testCannotCancelTerminalOrUnknownOrder();
    testRiskQuantityLimitIntegration();
    return 0;
}
