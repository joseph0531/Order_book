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
        processBuyOrder(new_book, trades);

    } else if(order.side == "Sell") {
        processSellOrder(new_book, trades);
    }
    return trades;
}


void OrderBook::printOrder() const {

    for(auto levelIt = buyLevels_.begin(); levelIt != buyLevels_.end(); ++levelIt) {
        const PriceLevel& orders = levelIt->second;

        for (auto orderIt = orders.begin(); orderIt != orders.end(); ++ orderIt) {
            std::cout << "id : " << orderIt->id << endl;
            std::cout << "side : " << orderIt->side << endl;
            std::cout << "price : " << orderIt->price << endl;
            std::cout << "quantity : " << orderIt->quantity << endl;
        }
    }
    
    for(auto levelIt = sellLevels_.begin(); levelIt != sellLevels_.end(); ++levelIt) {
        const PriceLevel& orders = levelIt->second;

        for (auto orderIt = orders.begin(); orderIt != orders.end(); ++ orderIt) {
            std::cout << "id : " << orderIt->id << endl;
            std::cout << "side : " << orderIt->side << endl;
            std::cout << "price : " << orderIt->price << endl;
            std::cout << "quantity : " << orderIt->quantity << endl;
        }
    }
}


bool OrderBook::cancelOrder(string side, int id) {
    if (side == "Buy") {
        for (auto levelIt = buyLevels_.begin(); levelIt != buyLevels_.end(); ++levelIt) {
            PriceLevel& orders = levelIt->second;

            for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
                if (orderIt->id == id) {
                    orders.erase(orderIt);
                    orderids_.erase(id);
                    if (orders.empty()) {
                        buyLevels_.erase(levelIt);
                    }
                    return true;
                }
            }
        }
    } else if (side == "Sell") {

        for (auto levelIt = sellLevels_.begin(); levelIt != sellLevels_.end(); ++levelIt) {
            PriceLevel& orders = levelIt->second;
            for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
                if (orderIt->id == id) {
                    orders.erase(orderIt);
                    orderids_.erase(id);
                    if (orders.empty()) {
                        sellLevels_.erase(levelIt);
                    }
                    return true;
                }
            }
        }
        
    }
    return false;
}


size_t OrderBook::orderCount() const {
    return orderids_.size();
}


bool OrderBook::canMatch(const Order& buy, const Order& sell) const {
    if (buy.price >= sell.price) {
        return true;
    } else {
        return false;
    }
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


void OrderBook::processBuyOrder(Order& incoming, std::vector<Trade>& trades) {

    while(incoming.quantity > 0) {
        Order* bestSell = getBestSellOrder();

        if (bestSell == nullptr) {
            break;
        }

        if (!canMatch(incoming, *bestSell)) {
            break;
        }
        trades.push_back(executeTrade(incoming, *bestSell, bestSell->price));


        if (bestSell->quantity == 0) {
            removeBestSellOrder();
        }
    }

    if (incoming.quantity > 0) {
        orderids_.insert(incoming.id);
        addBuyToLevel(incoming);
    }

}


void OrderBook::processSellOrder(Order& incoming, std::vector<Trade>& trades) {
    while (incoming.quantity > 0) {

        Order* bestbuy = getBestBuyOrder();

        if(bestbuy == nullptr) {
            break;
        }
        if(!canMatch(*bestbuy, incoming)) {
            break;
        }

        trades.push_back(executeTrade(*bestbuy, incoming, bestbuy->price));

        if(bestbuy->quantity == 0) {
            removeBestBuyOrder();
        }
    }
    if (incoming.quantity > 0) {
        orderids_.insert(incoming.id);
        addSellToLevel(incoming);

    }

    
}