#pragma once
#ifndef ORDERSYSTEM_HPP
#define ORDERSYSTEM_HPP

#include "bpt.hpp"
#include "mystl.hpp"
#include "trainsystem.hpp"

enum class OrderStatus {kSUCCESS, kPENDING, kREFUNDED};
struct Order {
    int time;
    string20 username;
    string20 trainid;
    TrainSystem::TicketInfo ticketinfo;
    int num;
    OrderStatus status;
    bool operator < (const Order &other) {
        return time < other.time;
    }
};

class OrderSystem {
private:
    BPlusTree<string20, Order> userorder;
    BPlusTree<string20, Order> trainorder;

public:
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
    bool Refund(const string20 &s, int n) {
        auto p = userorder.Find(s);
        if (p.size() < n) {
            return false;
        }
        auto order = p[n - 1];
        if (order.status == OrderStatus::kREFUNDED) {
            return false;
        }
        userorder.Remove(s, order);
        if (order.status == OrderStatus::kPENDING) {
            trainorder.Remove(s, order);
        }
        order.status = OrderStatus::kREFUNDED;
        userorder.Insert(s, order);
        return true;
    }
};

#endif