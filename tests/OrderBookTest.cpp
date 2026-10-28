#include <cassert>
#include "OrderBook.hpp"
#include <iostream>




void testCancelSecondOrder() {
    OrderBook book;

    book.addOrder({1, "Buy", 100, 10});
    book.addOrder({2, "Buy", 100, 10});

    assert(book.orderCount() == 2);

    bool cancelled = book.cancelOrder("Buy", 2);

    assert(cancelled);

    book.addOrder({2, "Buy", 100, 10});

    assert(book.orderCount() == 2);

}




void testBuyMatching() {
    OrderBook book;
    std::vector<Trade> trades;
    
    book.addOrder({1, "Buy", 105, 20});
    book.addOrder({2, "Buy", 103, 30});
    
    assert(book.orderCount() == 2);
    
    trades = book.addOrder({3, "Sell", 100, 40});
    
    
    assert(trades[0].price == 105);
    assert(trades[0].quantity == 20);
    assert(trades[1].price == 103);
    assert(trades[1].quantity == 20);
    assert(book.orderCount() == 1);
    
}


void testInvalidOrder() {
    OrderBook book;

    bool caught = false;


    try {
        book.addOrder({1, "Buy", -100, 20});
    } catch (const std::invalid_argument& e) {
        caught = true;

    }

    assert(caught);
    assert(book.orderCount() == 0);

}

void testInvalidQuantity() {
    OrderBook book;

    bool caught = false;

    try {
        book.addOrder({1, "Buy", 100, 0});
    } catch (const std::invalid_argument& e) {
        caught = true;

    }
    assert(caught);
    assert(book.orderCount() == 0);
}


void testInvalidSide() {
    OrderBook book;

    bool caught = false;

    try {
        book.addOrder({1, "Hello", 100, 1});
    } catch (const std::invalid_argument& e) {
        caught = true;

    }
    assert(caught);
    assert(book.orderCount() == 0);
}

void testDuplicateOrderId() {
    OrderBook book;
    bool caught = false;
    book.addOrder({1, "Buy", 100, 20});
    book.addOrder({2, "Sell", 102, 20});

    assert(book.orderCount() == 2);
    try {
        book.addOrder({1, "Buy", 101, 30});
    } catch (const std::invalid_argument& e){
        caught = true;
    }
    assert(caught);
    caught = false;
    try {
        book.addOrder({2, "Buy", 101, 20});
    } catch (std::invalid_argument& e) {
        caught = true;
    }
    assert(caught);
    book.addOrder({3, "Buy", 100, 30});
    assert(book.orderCount() == 3);

}

int main() {
    testBuyMatching();
    testInvalidOrder();
    testInvalidQuantity();
    testInvalidSide();
    testDuplicateOrderId();
    testCancelSecondOrder();
    return 0;
}


