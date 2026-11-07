#include "OrderBook.hpp"
#include <chrono>
#include <iostream>
#include "OrderManager.hpp"



void benchmarkInsert(int orderCount) {
    double avg_time = 0;
    for (int run = 0; run < 10; run++) {
        OrderBook book;

        auto start = std::chrono::steady_clock::now();
        for (int orderId = 1; orderId <= orderCount; orderId++) {
            book.addOrder({orderId, Side::Buy, 100, 10});
        }
        auto end = std::chrono::steady_clock::now();
        auto all_time = end - start;
        auto all_time_us = std::chrono::duration_cast<std::chrono::microseconds>(all_time);
        if (run > 0){
            avg_time += all_time_us.count();
        }
        std::cout << "ALL time : " << all_time_us.count() << std::endl;
    }
    avg_time = avg_time / 9;

    std::cout << "Avg Time: " << avg_time << std::endl;

    return;

}

void multibenchmark(int N) {
    double avg_time = 0;
    for (int run = 0; run < 10; run++) {
        OrderBook book;
        for (int i = 1; i <= N; i++) {
            book.addOrder({i, Side::Sell, i, 10});
        }
        auto start = std::chrono::steady_clock::now();
        std::vector<Trade> trades = book.addOrder({N + 1, Side::Buy, N, N *10});
        auto end = std::chrono::steady_clock::now();
        auto all_time = end - start;
        auto all_time_us = std::chrono::duration_cast<std::chrono::microseconds>(all_time);

        std::cout << "ALL time : " << all_time_us.count() << std::endl;
        if (run > 0){
            avg_time += all_time_us.count();
        }

        if (trades.size() != static_cast<std::size_t>(N)) {
            std::cerr << "Unexpected trade count\n";
        }
        if (book.orderCount() != 0) {
            std::cerr << "Order book is not empty\n";
        }

    }
    avg_time = avg_time / 9;

    std::cout << "Matching : " << N << " orders " << "Avg Time: " << avg_time << std::endl;

}

void cancelOrderbenchmark(int N) {
    double avg_time = 0;
    for (int run = 0; run < 10; run++) {
        OrderBook book;
        for (int i = 1; i <= N; i++) {
            book.addOrder({i, Side::Buy, 100 + i, 10});
        }
        auto start = std::chrono::steady_clock::now();
        for (int i = 1; i <= N; i++) {
            if(!book.cancelOrder(i)) {
                throw std::invalid_argument("cancel faild");
            }
        }
        auto end = std::chrono::steady_clock::now();
        auto time_ = end - start;
        auto all_time_us = std::chrono::duration_cast<std::chrono::microseconds>(time_);

        if (run > 0) {
            avg_time += all_time_us.count();
        }
        if (book.orderCount() != 0) {
            throw std::invalid_argument("order not 0");
        }
    }
    avg_time = avg_time / 9;
    std::cout << "Cancel : " << N << "average : " << avg_time << std::endl;
    return;
}





int main() {

    return 0;
}