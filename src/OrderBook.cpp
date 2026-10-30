#include "OrderBook.hpp"
#include <iostream>
#include <algorithm>

using namespace std;


vector<Trade> OrderBook::addOrder(const Order& order) {

    vector<Trade> trades;
    Order new_book = order;

    if (!isValidOrder(new_book)){
        throw invalid_argument("Invalid order");
    }
    if (hasOrderId(new_book.id)) {
        throw invalid_argument("Duplicate order ID");
    } 
    if (new_book.side == "Buy") {
        while(new_book.quantity > 0) {
            int best = findBestSellIndex();
            if (best == -1) {
                break;
            }
            bool OK_catch = canMatch(new_book, sellOrders_[best]);
            if ( OK_catch == false) {
                break;
            } 
            int OK_price = sellOrders_[best].price;

            Trade text = executeTrade(new_book, sellOrders_[best], OK_price);
            
            trades.push_back(text);

            if (sellOrders_[best].quantity == 0) {
                orderids_.erase(sellOrders_[best].id);
                sellOrders_.erase(sellOrders_.begin() + best);
            }
        }
        if (new_book.quantity > 0) {
            buyOrders_.push_back(new_book);
            orderids_.insert(new_book.id);
        }
    } else if(order.side == "Sell") {
        while(new_book.quantity > 0) {
            int best = findBestBuyIndex();
            if (best == -1) {
                break;
            }
            bool OK_catch = canMatch(buyOrders_[best], new_book);
            if ( OK_catch == false) {
                break;
            } 
            int OK_price = buyOrders_[best].price;

            Trade text = executeTrade(buyOrders_[best], new_book, OK_price);
            
            trades.push_back(text);

            if (buyOrders_[best].quantity == 0) {
                orderids_.erase(buyOrders_[best].id);
                buyOrders_.erase(buyOrders_.begin() + best);
                
            }
        }
        if (new_book.quantity > 0) {
            sellOrders_.push_back(new_book);
            orderids_.insert(new_book.id);
        }
    }
    return trades;
}


void OrderBook::printOrder() const {
    for (const Order &item : buyOrders_) {
        cout << "ID: " << item.id << endl;
        cout << "Side: " << item.side << endl;
        cout << "Price: " << item.price << endl;
        cout << "Quantity: " << item.quantity << endl;
    }
    for (const Order &item : sellOrders_) {
        cout << "ID: " << item.id << endl;
        cout << "Side: " << item.side << endl;
        cout << "Price: " << item.price << endl;
        cout << "Quantity: " << item.quantity << endl;
    }

}


bool OrderBook::cancelOrder(string side, int id) {
    if (side == "Buy") {
        for (int i = 0; i < buyOrders_.size(); i++) {
            if (buyOrders_[i].id == id) {
                orderids_.erase(buyOrders_[i].id);
                buyOrders_.erase(buyOrders_.begin() + i);
                return true;
            }
        }
        return false;
    } else if (side == "Sell") {
        for (int i = 0; i < sellOrders_.size(); i++) {
            if (sellOrders_[i].id == id) {
                orderids_.erase(sellOrders_[i].id);
                sellOrders_.erase(sellOrders_.begin() + i);
                return true;
            }
        }
        return false;
    }
    return false;
}


size_t OrderBook::orderCount() const {
    return buyOrders_.size() + sellOrders_.size();
}


bool OrderBook::canMatch(const Order& buy, const Order& sell) const {
    if (buy.price >= sell.price) {
        return true;
    } else {
        return false;
    }
} 


int OrderBook::findBestBuyIndex() const {
    if (buyOrders_.size() == 0) {
        return -1;
    }

    int best = 0;

    for (int i = 1; i < buyOrders_.size(); i++) {
        if (buyOrders_[i].price > buyOrders_[best].price) {
            best = i;
        }
    }
    return best;
}

int OrderBook::findBestSellIndex() const {
    if (sellOrders_.size() == 0) {
        return -1;
    }

    int best = 0;

    for (int i = 1; i < sellOrders_.size(); i++) {
        if (sellOrders_[i].price < sellOrders_[best].price) {
            best = i;
        }
    }
    return best;
}

Trade OrderBook::executeTrade(Order& buy, Order& sell, int tradePrice) {
    int min_quantity = min(buy.quantity, sell.quantity);

    buy.quantity -= min_quantity;
    sell.quantity -= min_quantity;

    Trade trade_;
    trade_.buyOrderId = buy.id;
    trade_.sellOrderId = sell.id;
    trade_.price = tradePrice;
    trade_.quantity = min_quantity;

    return trade_;

}


bool OrderBook::isValidOrder(const Order& order) const {
    if (order.side != "Buy" && order.side != "Sell") {
        return false;
    }
    if (order.price <= 0) {
        return false;
    }

    if (order.quantity <= 0) {
        return false;
    }

    return true;
}

bool OrderBook::hasOrderId(int id) const {

    if (orderids_.count(id) > 0) {
        return true;
    }
    return false;
}


void OrderBook::addBuyToLevel(const Order& order) {
    buyLevels_[order.price].push_back(order);
}

void OrderBook::addSellToLevel(const Order& order) {
    sellLevels_[order.price].push_back(order);
}

Order* OrderBook::getBestBuyOrder() {
    if (buyLevels_.size() == 0) {
        return nullptr;
    }
    map<int, PriceLevel, greater<int>>::iterator buy_high_price = buyLevels_.begin();

    PriceLevel& price_queue = buy_high_price->second;

    Order& earlylist_price = price_queue.front();

    return &earlylist_price;
}

Order* OrderBook::getBestSellOrder() {
    if(sellLevels_.size() == 0) {
        return nullptr;
    }

    map<int, PriceLevel>::iterator sell_low_price = sellLevels_.begin(); 

    PriceLevel& price_queue = sell_low_price->second;

    Order& earlylist_price = price_queue.front();

    return &earlylist_price;
}


void OrderBook::removeBestBuyOrder() {
    map<int, PriceLevel, greater<int>>::iterator buy_high_price = buyLevels_.begin();

    PriceLevel& price_queue = buy_high_price->second;

    Order& earlylist_price = price_queue.front();

    orderids_.erase(earlylist_price.id);

    price_queue.pop_front();

    if (price_queue.empty()) {
        buyLevels_.erase(buy_high_price);
    }
}

void OrderBook::removeBestSellOrder() {
    map<int, PriceLevel>::iterator sell_low_price = sellLevels_.begin();

    PriceLevel& price_queue = sell_low_price->second;

    Order& earlylist_price = price_queue.front();

    orderids_.erase(earlylist_price.id);

    price_queue.pop_front();

    if (price_queue.empty()) {
        sellLevels_.erase(sell_low_price);
    }

}