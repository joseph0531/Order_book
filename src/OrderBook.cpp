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
    if (new_book.side == Side::Buy) {
        processBuyOrder(new_book, trades);

    } else if(new_book.side == Side::Sell) {
        processSellOrder(new_book, trades);
    }
    return trades;
}

const char* OrderBook::sideToString(Side side) {
    switch (side) {
        case Side::Buy:
            return "Buy";

        case Side::Sell:
            return "Sell";
    }
    return "Unknown";
}



void OrderBook::printOrder() const {

    for(auto levelIt = buyLevels_.begin(); levelIt != buyLevels_.end(); ++levelIt) {
        const PriceLevel& orders = levelIt->second;

        for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
            std::cout << "id : " << orderIt->id << endl;
            std::cout << "side : " << sideToString(orderIt->side) << endl;
            std::cout << "price : " << orderIt->price << endl;
            std::cout << "quantity : " << orderIt->quantity << endl;
        }
    }
    
    for(auto levelIt = sellLevels_.begin(); levelIt != sellLevels_.end(); ++levelIt) {
        const PriceLevel& orders = levelIt->second;

        for (auto orderIt = orders.begin(); orderIt != orders.end(); ++orderIt) {
            std::cout << "id : " << orderIt->id << endl;
            std::cout << "side : " << sideToString(orderIt->side) << endl;
            std::cout << "price : " << orderIt->price << endl;
            std::cout << "quantity : " << orderIt->quantity << endl;
        }
    }
}


bool OrderBook::cancelOrder(int id) {

    auto locationIt = orderLocations_.find(id);
    if(locationIt == orderLocations_.end()) {
        return false;
    }
    auto find_order = locationIt->second;
    

    if(find_order.side == Side::Buy) {
        auto buy_price = buyLevels_.find(find_order.price);
        if (buy_price == buyLevels_.end()){
            return false;
        }
        PriceLevel& price_queue = buy_price->second;
    
        for (auto orderIt = price_queue.begin(); orderIt != price_queue.end(); ++orderIt) {
            if (orderIt->id == id) {
                price_queue.erase(orderIt);

                orderLocations_.erase(id);
                if (price_queue.empty()) {
                    buyLevels_.erase(buy_price);
                }
                return true;
            }
        }
    } else if(find_order.side == Side::Sell){
        auto sell_price = sellLevels_.find(find_order.price);
        if(sell_price == sellLevels_.end()) {
            return false;
        }
        PriceLevel& price_queue = sell_price->second;
    
        for (auto orderIt = price_queue.begin(); orderIt != price_queue.end(); ++orderIt) {
            if (orderIt->id == id) {
                price_queue.erase(orderIt);

                orderLocations_.erase(id);
                if (price_queue.empty()) {
                    sellLevels_.erase(sell_price);
                }
                return true;
            }
        }
    }
    return false;
}


size_t OrderBook::orderCount() const {
    return orderLocations_.size();
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
    if (order.side != Side::Buy && order.side != Side::Sell) {
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
    return orderLocations_.find(id) != orderLocations_.end();
}


void OrderBook::addBuyToLevel(const Order& order) {
    buyLevels_[order.price].push_back(order);


    OrderLocation location;

    location.side = order.side;
    location.price = order.price;

    orderLocations_.insert({order.id, location});
}

void OrderBook::addSellToLevel(const Order& order) {
    sellLevels_[order.price].push_back(order);


    OrderLocation location;

    location.side = order.side;
    location.price = order.price;

    orderLocations_.insert({order.id, location});
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

    orderLocations_.erase(earlylist_price.id);
    price_queue.pop_front();

    if (price_queue.empty()) {
        buyLevels_.erase(buy_high_price);
    }
}

void OrderBook::removeBestSellOrder() {
    map<int, PriceLevel>::iterator sell_low_price = sellLevels_.begin();

    PriceLevel& price_queue = sell_low_price->second;

    Order& earlylist_price = price_queue.front();

    orderLocations_.erase(earlylist_price.id);
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
        addSellToLevel(incoming);

    }

    
}