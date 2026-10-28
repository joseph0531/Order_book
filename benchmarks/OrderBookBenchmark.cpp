#include "OrderBook.hpp"
#include <chrono>
#include <iostream>

using namespace std;


double benchmarkInsert(int orderCount) {
    double avg_time = 0;
    for (int run = 0; run < 10; run++) {
        OrderBook book;

        auto start = chrono::steady_clock::now();
        for (int orderId = 1; orderId <= orderCount; orderId++) {
            book.addOrder({orderId, "Buy", 100, 10});
        }
        auto end = chrono::steady_clock::now();
        auto all_time = end - start;
        auto all_time_us = chrono::duration_cast<chrono::microseconds>(all_time);
        if (run > 0){
            avg_time += all_time_us.count();
        }
        cout << "ALL time : " << all_time_us.count() << endl;
    }
    avg_time = avg_time / 9;

    cout << "Avg Time: " << avg_time << endl;

    return 0;

}


int main() {

    benchmarkInsert(10000);
    benchmarkInsert(20000);
    benchmarkInsert(40000);
    return 0;
}