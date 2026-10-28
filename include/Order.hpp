#pragma once

#include <string>


struct Order {
    int id;
    std::string side;
    int price;
    int quantity;
};