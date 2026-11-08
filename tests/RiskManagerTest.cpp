#include <cassert>
#include <stdexcept>

#include "RiskManager.hpp"

void testValidOrderAndQuantityBoundary() {
    const RiskManager riskManager{RiskLimits{100}};

    const RiskCheckResult normalResult =
        riskManager.checkOrder({1, Side::Buy, 100, 50});
    assert(normalResult.approved);
    assert(normalResult.reason == RiskRejectReason::None);

    const RiskCheckResult boundaryResult =
        riskManager.checkOrder({2, Side::Sell, 100, 100});
    assert(boundaryResult.approved);
    assert(boundaryResult.reason == RiskRejectReason::None);
}

void testEachRejectReason() {
    const RiskManager riskManager{RiskLimits{100}};

    const RiskCheckResult invalidSide =
        riskManager.checkOrder({1, static_cast<Side>(999), 100, 10});
    assert(!invalidSide.approved);
    assert(invalidSide.reason == RiskRejectReason::InvalidSide);

    const RiskCheckResult invalidPrice =
        riskManager.checkOrder({2, Side::Buy, 0, 10});
    assert(!invalidPrice.approved);
    assert(invalidPrice.reason == RiskRejectReason::InvalidPrice);

    const RiskCheckResult invalidQuantity =
        riskManager.checkOrder({3, Side::Buy, 100, 0});
    assert(!invalidQuantity.approved);
    assert(invalidQuantity.reason == RiskRejectReason::InvalidQuantity);

    const RiskCheckResult overLimit =
        riskManager.checkOrder({4, Side::Buy, 100, 101});
    assert(!overLimit.approved);
    assert(overLimit.reason == RiskRejectReason::OrderQuantityLimitExceeded);
}

void testRejectRuleOrder() {
    const RiskManager riskManager{RiskLimits{100}};

    const RiskCheckResult result =
        riskManager.checkOrder({1, static_cast<Side>(999), 0, 0});
    assert(!result.approved);
    assert(result.reason == RiskRejectReason::InvalidSide);
}

void testInvalidRiskConfiguration() {
    bool zeroLimitRejected = false;
    try {
        const RiskManager riskManager{RiskLimits{0}};
        (void)riskManager;
    } catch (const std::invalid_argument&) {
        zeroLimitRejected = true;
    }
    assert(zeroLimitRejected);

    bool negativeLimitRejected = false;
    try {
        const RiskManager riskManager{RiskLimits{-1}};
        (void)riskManager;
    } catch (const std::invalid_argument&) {
        negativeLimitRejected = true;
    }
    assert(negativeLimitRejected);
}

int main() {
    testValidOrderAndQuantityBoundary();
    testEachRejectReason();
    testRejectRuleOrder();
    testInvalidRiskConfiguration();
    return 0;
}
