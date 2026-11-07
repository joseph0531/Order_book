# C++ Electronic Trading System

使用 C++17 與 CMake，從零建立一套模組化的電子交易系統。本專案以正確性、可測試性、可解釋的資料結構與低延遲設計概念為優先，逐步整合 Order Management、Risk、Matching Engine、Gateway、Market Data、Execution Report 與 Persistence。

> 目前是教育與作品集專案，不是可直接部署到真實市場的 production exchange。

## 1. 系統最終資料流

```text
Trading Client
      │ TCP request
      ▼
Order Gateway
      │ 解析、驗證訊息格式、管理 session
      ▼
Trading Engine / Coordinator
      ├──► Risk Manager ── 拒絕 ──► Execution Report
      │
      ├──► Order Manager ── 建立與更新 OrderState
      │
      └──► Matching Engine / OrderBook
                │
                ├── Trade events ──► Order Manager
                ├── Book events  ──► Market Data Publisher
                └── Journal      ──► Persistence / Recovery

Order Manager
      └── Execution Reports ──► Gateway ──► Client
```

核心原則：各模組只處理自己的責任，透過 request、event 與 report 整合，不讓網路、風控、撮合與儲存邏輯混在同一個 class。

## 2. 模組責任

### 2.1 Domain Models

共用且不依賴特定模組的資料型態：

- `Side`：`Buy`、`Sell`
- `Order`：送入 Matching Engine 的訂單資料
- `Trade`：一次撮合結果
- `OrderStatus`：訂單生命週期狀態
- `OrderState`：Order Manager 保存的完整訂單狀態
- 未來：`NewOrderRequest`、`CancelRequest`、`ExecutionReport`、`BookUpdate`

### 2.2 Order Gateway

負責系統邊界與 TCP 通訊：

- 接受 Client 連線
- 解析 wire message
- 將 message 轉成 typed request
- 管理 client/session
- 將 Execution Report 與 Market Data 傳回 Client

Gateway 不負責撮合，也不直接修改 OrderBook。

### 2.3 Risk Manager

下單進入 Matching Engine 前執行風控：

- quantity 與 price 限制
- 單筆 notional 限制
- client 最大未完成訂單數
- position／exposure 限制
- kill switch

風控失敗時建立 `Rejected` Execution Report，訂單不得進入 OrderBook。

### 2.4 Order Manager

負責訂單生命週期與查詢：

- 建立 `OrderState`
- 防止重複 order ID
- 套用 Trade，更新 remaining quantity
- 管理 `Accepted`、`PartiallyFilled`、`Filled`、`Cancelled`、`Rejected`
- 產生 Execution Report 所需資料
- 即使 Order 已離開 OrderBook，仍保留其狀態

Order Manager 回答「這張訂單經歷了什麼」，OrderBook 回答「現在有哪些訂單可以撮合」。

### 2.5 Matching Engine / OrderBook

負責價格與時間優先撮合：

- Buy best price：最高 bid
- Sell best price：最低 ask
- 同價格：FIFO
- crossing condition：`buy.price >= sell.price`
- trade price：resting order price
- partial fill 與 multiple fills
- cancel resting order
- 維護 `order ID -> side + price` 索引

目前核心資料結構：

```cpp
using PriceLevel = std::deque<Order>;

std::map<int, PriceLevel, std::greater<int>> buyLevels_;
std::map<int, PriceLevel> sellLevels_;
std::unordered_map<int, OrderLocation> orderLocations_;
```

概念結構：

```text
Buy levels（高到低）          Sell levels（低到高）
105 -> [A, B]                106 -> [D, E]
103 -> [C]                   108 -> [F]
```

`map` 管價格優先，`deque` 管同價格 FIFO，location index 協助取消訂單。

### 2.6 Execution Reporting

把系統內部狀態轉成 Client 可理解的事件：

- Order Accepted
- Order Rejected
- Partial Fill
- Fill
- Cancel Accepted
- Cancel Rejected

未來每份 report 應包含 order ID、狀態、成交數量、剩餘數量、成交價格與拒絕原因等欄位。

### 2.7 Market Data

由 OrderBook 變化產生行情事件：

- best bid / best ask
- price-level quantity
- order book snapshot
- incremental book updates
- trade ticks

Market Data Publisher 不應直接成為撮合邏輯的一部分；Matching Engine 只產生事件。

### 2.8 Persistence & Recovery

負責讓系統在重啟後恢復：

- append-only journal
- 記錄 accepted order、cancel、trade 與 reject
- 啟動時 replay journal
- 定期 snapshot，縮短 recovery 時間
- 驗證 replay 後的 OrderState 與 OrderBook invariant

第一版會先追求 deterministic replay，再考慮高效 binary format。

### 2.9 Low-Latency Optimization

先測量，再優化：

- Release `-O3` benchmark
- insertion、matching、cancel 分開測量
- median、P95、P99
- 減少 allocation 與資料複製
- 避免 hot path 使用字串
- cache-friendly layout
- object pool / memory resource
- single-writer event loop
- 模組間 bounded queue

多執行緒不是第一步；先建立正確、可重現、單執行緒 deterministic core。

## 3. 重要資料模型

### 3.1 Order

`Order` 是 Matching Engine 使用的訂單資料。

```cpp
struct Order {
    int id;
    Side side;
    int price;
    int quantity;
};
```

OrderBook 內的 `quantity` 表示目前可成交的剩餘數量。

### 3.2 Trade

```cpp
struct Trade {
    int buyOrderId;
    int sellOrderId;
    int price;
    int quantity;
};
```

一筆 incoming order 可能產生多筆 Trade。

### 3.3 OrderState

`OrderState` 是下一階段 Order Management 要建立的生命週期資料：

```cpp
enum class OrderStatus {
    PendingNew,
    Accepted,
    PartiallyFilled,
    Filled,
    Cancelled,
    Rejected
};

struct OrderState {
    int id;
    Side side;
    int price;
    int originalQuantity;
    int remainingQuantity;
    OrderStatus status;
};
```

不另外保存 filled quantity：

```text
filledQuantity = originalQuantity - remainingQuantity
```

避免 original、remaining、filled 三份資料互相不同步。

## 4. Order 生命週期

```text
PendingNew
├── Rejected
└── Accepted
    ├── PartiallyFilled
    │   ├── Filled
    │   └── Cancelled
    ├── Filled
    └── Cancelled
```

主要 invariant：

```text
Accepted:
remainingQuantity == originalQuantity

PartiallyFilled:
0 < remainingQuantity < originalQuantity

Filled:
remainingQuantity == 0

Cancelled:
不可再進入撮合

Rejected:
不可進入 OrderBook
```

範例：quantity 100 的 Buy order。

```text
收到 request      original=100, remaining=100, PendingNew
系統接受          original=100, remaining=100, Accepted
成交 30           original=100, remaining=70,  PartiallyFilled
再成交 70         original=100, remaining=0,   Filled
```

## 5. 模組整合時序

### New Order

```text
1. Gateway 收到 NewOrderRequest
2. Coordinator 建立 PendingNew 狀態
3. Risk Manager 執行 pre-trade checks
4. 拒絕：OMS -> Rejected，產生 ExecutionReport
5. 接受：OMS -> Accepted，送入 OrderBook
6. OrderBook 回傳零到多筆 Trade
7. OMS 對每筆 Trade 更新 Buy 與 Sell 的 OrderState
8. 產生 PartialFill / Fill reports
9. 發布 Market Data events
10. 所有事件寫入 journal
```

### Cancel Order

```text
1. Gateway 收到 CancelRequest
2. Coordinator 查詢 OMS 狀態
3. Filled / Cancelled / Rejected 不可再次取消
4. 呼叫 OrderBook.cancelOrder(id)
5. 成功：OMS -> Cancelled，產生 Cancel report
6. 失敗：產生 CancelRejected report
7. 發布 book update 並寫入 journal
```

## 6. 目前進度

### 已完成

- [x] C++17 / CMake 專案
- [x] Limit order add
- [x] Buy / Sell matching
- [x] Partial fill 與 multiple fills
- [x] Resting-order trade price
- [x] Price-Time Priority
- [x] Cancel by ID
- [x] `ID -> side + price` cancel index
- [x] Duplicate ID validation
- [x] `Side` enum 與非法 enum 防禦
- [x] insertion benchmark
- [x] matching benchmark
- [x] cancel benchmark

### 目前階段

- [ ] `OrderState.hpp`
- [ ] `OrderManager` 第一版
- [ ] Trade 驅動的狀態更新
- [ ] Cancel 狀態更新
- [ ] Execution Report

### 後續階段

- [ ] Risk Manager
- [ ] Trading Engine / Coordinator
- [ ] Market Data events
- [ ] Journal 與 deterministic replay
- [ ] TCP Order Gateway / Client
- [ ] 執行緒模型與 queue
- [ ] profiling、percentiles 與記憶體優化

## 7. 建議目錄演進

目前不需要一次搬完；隨模組完成逐步演進：

```text
Order_Book/
├── include/
│   ├── Order.hpp
│   ├── Trade.hpp
│   ├── OrderState.hpp
│   ├── OrderBook.hpp
│   ├── OrderManager.hpp
│   ├── RiskManager.hpp
│   ├── ExecutionReport.hpp
│   └── TradingEngine.hpp
├── src/
│   ├── OrderBook.cpp
│   ├── OrderManager.cpp
│   ├── RiskManager.cpp
│   ├── TradingEngine.cpp
│   └── main.cpp
├── gateway/
│   ├── OrderGateway.hpp
│   ├── OrderGateway.cpp
│   └── TradingClient.cpp
├── persistence/
│   ├── Journal.hpp
│   └── Journal.cpp
├── tests/
├── benchmarks/
└── CMakeLists.txt
```

## 8. Build、Test、Benchmark

### 開發建置與測試

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

### Release benchmark

```bash
cmake -S . -B build-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
./build-release/orderbook_benchmark
```

Benchmark 必須記錄 workload、build mode、compiler flags、重複次數與統計方式，避免只報告單次最佳數字。

## 9. 下一個開發任務

建立 `include/OrderState.hpp`：

1. 定義 `OrderStatus`。
2. 定義 `OrderState`。
3. 不修改 OrderBook。
4. 確認現有 build 與 tests 仍通過。
5. 下一步再建立 `OrderManager`，先實作 accepted order 的登記與查詢。
