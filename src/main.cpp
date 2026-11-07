#include "OrderBook.hpp"



int main() {
    OrderBook book;

    book.addOrder({1, Side::Sell, 100, 10});

    book.printOrder();
    
    return 0;
}