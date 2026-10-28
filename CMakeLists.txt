
##專案至少要這個版本以上
cmake_minimum_required(VERSION 3.16)

##定義專案名稱並告訴CMake這是C++
project(OrderBook LANGUAGES CXX)

##設定使用C++17 並要求編譯器必須支援這版本
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

##把.cpp編譯連結成一個叫做orderbook的執行檔
add_executable(orderbook
    src/main.cpp
    src/OrderBook.cpp
)

##告訴編譯器當看到hpp請到以下目錄去尋找
target_include_directories(orderbook
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(orderbook_benchmark
    benchmarks/OrderBookBenchmark.cpp
    src/OrderBook.cpp
)

target_include_directories(orderbook_benchmark PRIVATE include)

add_executable(orderbook_test
    tests/OrderBookTest.cpp
    src/OrderBook.cpp    
)

target_include_directories(orderbook_test PRIVATE include)

enable_testing()

add_test(
        NAME OrderBookTest
        COMMAND orderbook_test
)


##cmake -S . -B build
##cmake --build build