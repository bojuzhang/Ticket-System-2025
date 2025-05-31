#pragma once
#ifndef ORDERSYSTEM_HPP
#define ORDERSYSTEM_HPP

#include "bpt.hpp"
#include "trainsystem.hpp"

enum class OrderStatus {kSUCCESS, kPENDING, kREFUNDED};
struct Order {
    int time;
    string20 username;
    string20 trainid;
    TrainSystem::OrderInfo orderinfo;
    string20 from, to;
    int num;
    OrderStatus status;
    bool operator < (const Order &other) {
        return time < other.time;
    }
    bool operator > (const Order &other) {
        return time > other.time;
    }
    bool operator == (const Order &other) {
        return time == other.time;
    }
    bool operator <= (const Order &other) {
        return !((*this) > other);
    }
    bool operator >= (const Order &other) {
        return !((*this) < other);
    }
    bool operator != (const Order &other) {
        return !((*this) == other);
    }
};

class OrderSystem {
private:
    BPlusTree<string20, Order> userorder{"userorder"};
    BPlusTree<string20, Order> trainorder{"trainorder"};

public:
    void Clear() {
        userorder.Clear();
        trainorder.Clear();
    }
    void AddOrder(const Order &order) {
        userorder.Insert(order.username, order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Insert(order.trainid, order);
        }
    }
    void DelOrder(const Order &order) {
        userorder.Remove(order.username, order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Remove(order.trainid, order);
        }
    }
    vector<Order> GetRefund(const string20 &trainid) {
        return trainorder.Find(trainid);
    }
    bool Refund(Order order) {
        if (order.status == OrderStatus::kREFUNDED) {
            return false;
        }
        userorder.Remove(order.username, order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Remove(order.trainid, order);
        }
        order.status = OrderStatus::kREFUNDED;
        userorder.Insert(order.username, order);
        return true;
    }
    vector<Order> QueryOrder(const string20 &username) {
        return  userorder.Find(username);
    }
};

#endif