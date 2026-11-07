#include <cassert>
#include "OrderManager.hpp"
#include <iostream>


int main () {
    OrderManager order_manage;
    Order order, order_1, order_2;

    order.id = 1;
    order.price = 101;
    order.quantity = 100;
    order.side = Side::Buy;

    order_1.id = 2;
    order_1.price = 101;
    order_1.quantity = 100;
    order_1.side = Side::Buy;

    order_2.id = 3;
    order_2.price = 101;
    order_2.quantity = 100;
    order_2.side = Side::Buy;   

    order_manage.createOrder(order);

    order_manage.acceptOrder(1);

    assert(order_manage.cancelOrder(1) == true);

    assert(order_manage.getOrder(1)->remainingQuantity == 100);
    assert(order_manage.getOrder(1)->status == OrderStatus::Cancelled);
    assert(order_manage.cancelOrder(1) == false);
    assert(order_manage.applyFill(1, 30) == false);

    order_manage.createOrder(order_1);

    order_manage.acceptOrder(2);

    order_manage.applyFill(2, 30);
    assert(order_manage.cancelOrder(2) == true);

    assert(order_manage.getOrder(2)->status == OrderStatus::Cancelled);
    assert(order_manage.getOrder(2)->remainingQuantity == 70);
    assert(order_manage.applyFill(2, 30) == false);


    order_manage.createOrder(order_2);

    assert(order_manage.cancelOrder(3) == false);
    assert(order_manage.cancelOrder(5555) == false);


    

    



}