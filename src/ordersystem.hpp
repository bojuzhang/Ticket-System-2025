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
    string30 from, to;
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
    BPlusTree<ull, Order> userorder{"userorder"};
    BPlusTree<ull, Order> trainorder{"trainorder"};

public:
    void Clear() {
        userorder.Clear();
        trainorder.Clear();
    }
    void AddOrder(const Order &order) {
        userorder.Insert(hash(order.username), order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Insert(hash(order.trainid), order);
        }
    }
    void DelOrder(const Order &order) {
        userorder.Remove(hash(order.username), order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Remove(hash(order.trainid), order);
        }
    }
    vector<Order> GetRefund(const string20 &trainid) {
        return trainorder.Find(hash(trainid));
    }
    bool Refund(Order order) {
        if (order.status == OrderStatus::kREFUNDED) {
            return false;
        }
        userorder.Remove(hash(order.username), order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Remove(hash(order.trainid), order);
        }
        order.status = OrderStatus::kREFUNDED;
        userorder.Insert(hash(order.username), order);
        return true;
    }
    vector<Order> QueryOrder(const string20 &username) {
        return  userorder.Find(hash(username));
    }
};

#endif