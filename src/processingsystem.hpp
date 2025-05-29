#pragma once
#include <string>
#ifndef PROCESSINGSYSTEM_HPP
#define PROCESSINGSYSTEM_HPP

#include "usersystem.hpp"
#include "trainsystem.hpp"
#include "ordersystem.hpp"
#include <iostream>


class ProcessingSystem {
private:
    UserSystem usersys;
    TrainSystem trainsys;
    OrderSystem ordersys;

    vector<std::string> GetToken() {
        vector<std::string> v;
        std::string s, t;
        std::getline(std::cin, s);
        for (auto c : s) {
            if (isspace(c)) {
                if (!t.empty()) {
                    v.push_back(t);
                    t = "";
                }
                continue;
            }
            t += c;
        }
        if (!t.empty()) {
            v.push_back(t);
            t = "";
        }
        return v;
    }

    void AddUser() {

    }
    void Login() {

    }
    void Logout() {

    }
    void QueryProfile() {

    }
    void ModifyProfile() {

    }
    void AddTrain() {

    }
    void DeleteTrain() {

    }
    void ReleaseTrain() {

    }
    void QueryTrain() {

    }
    void QueryTicket() {

    }
    void QueryTransfer() {

    }
    void BuyTicket() {

    }
    void QueryOrder() {

    }
    void RefundTicket() {

    }
    void Clean() {

    }
    void Exit() {

    }

public:
    void Run() {
        std::string times, op;
        while (std::cin >> times >> op) {
            int timestamp = std::stoi(times.substr(1, times.size() - 2));
            if (op == "add_user") {
                AddUser();
            } else if (op == "login") {
                Login();
            } else if (op == "logout") {
                Logout();
            } else if (op == "query_profile") {
                QueryProfile();
            } else if (op == "modify_profile") {
                ModifyProfile();
            } else if (op == "add_train") {
                AddTrain();
            } else if (op == "delete_train") {
                DeleteTrain();
            } else if (op == "release_train") {
                ReleaseTrain();
            } else if (op == "query_train") {
                QueryTrain();
            } else if (op == "query_ticket") {
                QueryTicket();
            } else if (op == "query_transfer") {
                QueryTransfer();
            } else if (op == "buy_ticket") {
                BuyTicket();
            } else if (op == "query_order") {
                QueryOrder();
            } else if (op == "refund_ticket") {
                RefundTicket();
            } else if (op == "clean") {
                Clean();
            } else if (op == "exit") {
                Exit();
            }
        }
    }
};


#endif