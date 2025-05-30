#pragma once
#ifndef PROCESSINGSYSTEM_HPP
#define PROCESSINGSYSTEM_HPP

#include "usersystem.hpp"
#include "trainsystem.hpp"
#include "ordersystem.hpp"
#include <iostream>
#include <string>

class ProcessingSystem {
private:
    UserSystem usersys;
    TrainSystem trainsys;
    OrderSystem ordersys;

    bool has_user;

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

    std::string ToString2(int x) {
        if (x == -1) {
            return "xx";
        }
        std::string s = std::to_string(x);
        if (x < 10) {
            s = "0" + s;
        }
        return s;
    }

    void AddUser() {
        User new_user;
        auto s = GetToken();
        string20 cur_username;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-c") {
                cur_username = s[i + 1];
            } else if (s[i] == "-u") {
                new_user.username = s[i + 1];
            } else if (s[i] == "-p") { 
                new_user.password = s[i + 1];
            } else if (s[i] == "-n") {
                new_user.name = s[i + 1];
            } else if (s[i] == "-m") { 
                new_user.mailaddr = s[i + 1];
            } else if (s[i] == "-g") { 
                new_user.privilege = std::stoi(s[i + 1]);
            }
        }
        if (!has_user) {
            new_user.privilege = 10;
            usersys.AddUser(new_user);
            has_user = 1;
            std::cout << 0 << "\n";
            return;
        }
        auto p = usersys.QueryUser(cur_username);
        if (!p.second || !p.first.loggined || p.first.privilege <= new_user.privilege) {
            std::cout << -1 << "\n";
            return;
        }
        auto tmp = usersys.AddUser(new_user);
        std::cout << (tmp ? 0 : -1) << "\n";
    }
    void Login() {
        auto s = GetToken();
        string20 username;
        string30 password;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-u") {
                username = s[i + 1];
            } else if (s[i] == "-p") {
                password = s[i + 1];
            }
        }
        auto p = usersys.QueryUser(username);
        if (p.second || p.first.loggined) {
            std::cout << -1 << "\n";
            return;
        }
        usersys.Login(p.first);
        std::cout << 0 << "\n";
    }
    void Logout() {
        auto s = GetToken();
        string20 username = s[1];
        auto p = usersys.QueryUser(username);
        if (p.second || p.first.loggined) {
            std::cout << -1 << "\n";
            return;
        }
        usersys.Logout(p.first);
        std::cout << 0 << "\n";
    }
    void QueryProfile() {
        auto s = GetToken();
        string20 username;
        string20 cur_username;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-u") {
                username = s[i + 1];
            } else if (s[i] == "-c") {
                cur_username = s[i + 1];
            }
        }
        auto p = usersys.QueryUser(username);
        auto curp = usersys.QueryUser(cur_username);
        if (!p.second || !curp.second) {
            std::cout << -1 << "\n";
            return;
        }
        if (!curp.first.loggined || (curp.first.privilege <= p.first.privilege && username != cur_username)) {
            std::cout << -1 << "\n";
            return;
        }
        std::cout << p.first.username << " " << p.first.name << " " << p.first.mailaddr << " " << p.first.privilege << "\n";
    }
    void ModifyProfile() {
        auto s = GetToken();
        string20 username;
        string20 cur_username;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-u") {
                username = s[i + 1];
            } else if (s[i] == "-c") {
                cur_username = s[i + 1];
            }
        }
        auto p = usersys.QueryUser(username);
        auto curp = usersys.QueryUser(cur_username);
        if (!p.second || !curp.second) {
            std::cout << -1 << "\n";
            return;
        }
        if (!curp.first.loggined || (curp.first.privilege <= p.first.privilege && username != cur_username)) {
            std::cout << -1 << "\n";
            return;
        }
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-g") {
                int priv = std::stoi(s[i + 1]);
                if (curp.first.privilege <= priv) {
                    std::cout << -1 << "\n";
                    return;
                }
                usersys.ModifyPrivilege(p.first, priv);
                p.first.privilege = priv;
            }
        }
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-p") {
                usersys.ModifyPassword(p.first, s[i + 1]);
                p.first.password = s[i + 1];
            } else if (s[i] == "-n") {
                usersys.ModifyName(p.first, s[i + 1]);
                p.first.name = s[i + 1];
            } else if (s[i] == "-m") {
                usersys.ModifyMail(p.first, s[i + 1]);
                p.first.mailaddr = s[i + 1];
            }
        }
        std::cout << p.first.username << " " << p.first.name << " " << p.first.mailaddr << " " << p.first.privilege << "\n";
    }
    void AddTrain() {
        auto s = GetToken();
        Train train;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-i") {
                train.trainid = s[i + 1];
            } else if (s[i] == "-n") {
                train.stationnum = std::stoi(s[i + 1]);
            } else if (s[i] == "-m") {
                train.seatnum = std::stoi(s[i + 1]);
            } else if (s[i] == "-s") {
                int n = s[i + 1].size();
                for (int j = 0; j < n; j++) {
                    int len = 0;
                    while (j + len < n && s[i + 1][j + len] != '|') {
                        len++;
                    }
                    train.stations.push_back(s[i + 1].substr(j, len));
                    j += len;
                }
            } else if (s[i] == "-p") {
                int n = s[i + 1].size();
                for (int j = 0; j < n; j++) {
                    int len = 0;
                    while (j + len < n && s[i + 1][j + len] != '|') {
                        len++;
                    }
                    train.prices.push_back(std::stoi(s[i + 1].substr(j, len)));
                    j += len;
                }
            } else if (s[i] == "-x") {
                train.starttime.first = std::stoi(s[i + 1].substr(0, 2));
                train.starttime.second = std::stoi(s[i + 1].substr(3, 2));
            } else if (s[i] == "-t") {
                int n = s[i + 1].size();
                for (int j = 0; j < n; j++) {
                    int len = 0;
                    while (j + len < n && s[i + 1][j + len] != '|') {
                        len++;
                    }
                    train.traveltimes.push_back(std::stoi(s[i + 1].substr(j, len)));
                    j += len;
                }
            } else if (s[i] == "-o") {
                if (s[i + 1][0] == '_') {
                    continue;
                }
                int n = s[i + 1].size();
                for (int j = 0; j < n; j++) {
                    int len = 0;
                    while (j + len < n && s[i + 1][j + len] != '|') {
                        len++;
                    }
                    train.stopovertimes.push_back(std::stoi(s[i + 1].substr(j, len)));
                    j += len;
                }
            } else if (s[i] == "-d") {
                train.saledates.first.first = std::stoi(s[i + 1].substr(0, 2));
                train.saledates.first.second = std::stoi(s[i + 1].substr(3, 2));
                train.saledates.second.first = std::stoi(s[i + 1].substr(6, 2));
                train.saledates.second.second = std::stoi(s[i + 1].substr(9, 2));
            } else if (s[i] == "-y") {
                train.type = s[i + 1][0];
            }
        }
        auto tmp = trainsys.AddTrain(train);
        std::cout << (tmp ? 0 : -1) << "\n";
    }
    void DeleteTrain() {
        auto s = GetToken();
        auto tmp = trainsys.DeleteTrain(s[1]);
        std::cout << (tmp ? 0 : -1) << "\n";
    }
    void ReleaseTrain() {
        auto s = GetToken();
        auto tmp = trainsys.ReleaseTrain(s[1]);
        std::cout << (tmp ? 0 : -1) << "\n";
    }
    void QueryTrain() {
        auto s = GetToken();
        string20 trainid;
        pair<int, int> date;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-i") {
                trainid = s[i + 1];
            } else if (s[i] == "-d") {
                date.first = std::stoi(s[i + 1].substr(0, 2));
                date.second = std::stoi(s[i + 1].substr(3, 2));
            }
        }
        auto [ans, type] = trainsys.QueryTrain(trainid, date);
        if (!ans.size()) {
            std::cout << -1 << "\n";
            return;
        }
        std::cout << trainid << " " << type << "\n";
        for (auto p : ans) {
            std::cout << p.station << " ";
            std::cout << ToString2(p.arriving[0]) << "-" << ToString2(p.arriving[1]) << " ";
            std::cout << ToString2(p.arriving[2]) << ":" << ToString2(p.arriving[3]) << " -> ";
            std::cout << ToString2(p.leaving[0]) << "-" << ToString2(p.leaving[1]) << " ";
            std::cout << ToString2(p.leaving[2]) << ":" << ToString2(p.leaving[3]) << " ";
            std::cout << p.price << " " << (p.seat == -1 ? "x" : std::to_string(p.seat)) << "\n";
        }
    }
    void QueryTicket() {
        TrainSystem::TicketOrder order = TrainSystem::TicketOrder::kTIME;
        auto s = GetToken();
        pair<int, int> date;
        string20 from, to;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-s") {
                from = s[i + 1];
            } else if (s[i] == "-d") {
                date.first = std::stoi(s[i + 1].substr(0, 2));
                date.second = std::stoi(s[i + 1].substr(3, 2));
            } else if (s[i] == "-t") {
                to = s[i + 1];
            } else if (s[i] == "-p") {
                if (s[i + 1] == "cost") {
                    order = TrainSystem::TicketOrder::kCOST;
                } else if (s[i + 1] == "time") {
                    order = TrainSystem::TicketOrder::kTIME;
                }
            }
        }
        auto ans = trainsys.QueryTicket(from, to, date, order);
        std::cout << ans.size() << "\n";
        for (auto p : ans) {
            std::cout << p.trainid << " ";
            std::cout << p.from << " ";
            std::cout << ToString2(p.leaving[0]) << "-" << ToString2(p.leaving[1]) << " ";
            std::cout << ToString2(p.leaving[2]) << ":" << ToString2(p.leaving[3]) << " -> ";
            std::cout << p.to;
            std::cout << ToString2(p.arriving[0]) << "-" << ToString2(p.arriving[1]) << " ";
            std::cout << ToString2(p.arriving[2]) << ":" << ToString2(p.arriving[3]) << " ";
            std::cout << p.price << " " << p.seat << "\n";
        }
    }
    void QueryTransfer() {
        TrainSystem::TicketOrder order = TrainSystem::TicketOrder::kTIME;
        auto s = GetToken();
        pair<int, int> date;
        string20 from, to;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-s") {
                from = s[i + 1];
            } else if (s[i] == "-d") {
                date.first = std::stoi(s[i + 1].substr(0, 2));
                date.second = std::stoi(s[i + 1].substr(3, 2));
            } else if (s[i] == "-t") {
                to = s[i + 1];
            } else if (s[i] == "-p") {
                if (s[i + 1] == "cost") {
                    order = TrainSystem::TicketOrder::kCOST;
                } else if (s[i + 1] == "time") {
                    order = TrainSystem::TicketOrder::kTIME;
                }
            }
        }
        auto [ans, has_ans] = trainsys.QueryTransfer(from, to, date, order);
        if (!has_ans) {
            std::cout << 0 << "\n";
            return;
        }
        for (auto p : {ans.first, ans.second}) {
            std::cout << p.trainid << " ";
            std::cout << p.from << " ";
            std::cout << ToString2(p.leaving[0]) << "-" << ToString2(p.leaving[1]) << " ";
            std::cout << ToString2(p.leaving[2]) << ":" << ToString2(p.leaving[3]) << " -> ";
            std::cout << p.to << " ";
            std::cout << ToString2(p.arriving[0]) << "-" << ToString2(p.arriving[1]) << " ";
            std::cout << ToString2(p.arriving[2]) << ":" << ToString2(p.arriving[3]) << " ";
            std::cout << p.price << " " << p.seat << "\n";
        }
    }
    void BuyTicket(int timestamp) {
        auto s = GetToken();
        string20 username;
        string20 trainid;
        pair<int, int> date;
        string20 from, to;
        int n;
        bool q = false;
        for (int i = 0; i < static_cast<int>(s.size()); i += 2) {
            if (s[i] == "-f") {
                from = s[i + 1];
            } else if (s[i] == "-d") {
                date.first = std::stoi(s[i + 1].substr(0, 2));
                date.second = std::stoi(s[i + 1].substr(3, 2));
            } else if (s[i] == "-t") {
                to = s[i + 1];
            } else if (s[i] == "-u") {
                username = s[i + 1];
            } else if (s[i] == "-n") {
                n = std::stoi(s[i + 1]);
            } else if (s[i] == "-q") {
                q = (s[i + 1] == "false" ? false : true);
            } else if (s[i] == "-i") {
                trainid = s[i + 1];
            }
        }
        auto p = usersys.QueryUser(username);
        if (!p.second || !p.first.loggined) {
            std::cout << -1 << "\n";
            return;
        }
        auto [orderinfo, costs] = trainsys.BuyTickets(trainid, date, from, to, n);
        Order order;
        order.orderinfo = orderinfo;
        order.time = timestamp;
        order.username = username;
        order.trainid = trainid;
        order.from = from, order.to = to;
        if (costs > 0) {
            order.status = OrderStatus::kSUCCESS;
            ordersys.AddOrder(order);
            std::cout << costs << "\n";
        } else if (costs == 0) {
            if (q) {
                order.status = OrderStatus::kPENDING;
                ordersys.AddOrder(order);
                std::cout << "queue\n";
            } else {
                std::cout << -1 << "\n";
            }
        }
    }
    void QueryOrder() {
        auto s = GetToken();
        string20 username = s[1];
        auto [user, flag] = usersys.QueryUser(username);
        if (!flag || !user.loggined) {
            std::cout << -1 << "\n";
            return;
        }
        auto ans = ordersys.QueryOrder(username);
        std::cout << ans.size() << "\n";
        for (auto p : ans) {
            std::cout << "[" << (p.status == OrderStatus::kPENDING ? "pending\n" : (p.status == OrderStatus::kSUCCESS ? "success\n" : "refunded\n")) << "] ";
            std::cout << p.trainid << " " << p.from << " ";
            std::cout << ToString2(p.orderinfo.leaving[0]) << "-" << ToString2(p.orderinfo.leaving[1]) << " ";
            std::cout << ToString2(p.orderinfo.leaving[2]) << ":" << ToString2(p.orderinfo.leaving[3]) << " -> ";
            std::cout << p.to << " ";
            std::cout << ToString2(p.orderinfo.arriving[0]) << "-" << ToString2(p.orderinfo.arriving[1]) << " ";
            std::cout << ToString2(p.orderinfo.arriving[2]) << ":" << ToString2(p.orderinfo.arriving[3]) << " ";
            std::cout << p.orderinfo.price << " " << p.num << "\n";
        }
    }
    void RefundTicket() {
        auto s = GetToken();
        string20 username;
        int n = 1;
        for (int i = 0; i < static_cast<int>(s.size()); i++) {
            if (s[i] == "-u") {
                username = s[i + 1];
            } else if (s[i] == "-n") {
                n = std::stoi(s[i + 1]);
            }
        }
        auto [user, flag] = usersys.QueryUser(username);
        if (!flag || !user.loggined) {
            std::cout << -1 << "\n";
            return;
        }
        auto p = ordersys.QueryOrder(username);
        if (n > p.size()) {
            std::cout << -1 << "\n";
            return;
        }
        auto order = p[n - 1];
        if (order.status == OrderStatus::kSUCCESS) {
            trainsys.BuyTickets(order.trainid, {order.orderinfo.arriving[0], order.orderinfo.arriving[1]}, order.from, order.to, -order.num);
            auto res = ordersys.GetRefund(order.trainid);
            for (auto q : res) {
                auto [orderinfo, costs] = trainsys.BuyTickets(q.trainid, {q.orderinfo.arriving[0], q.orderinfo.arriving[1]}, q.from, q.to, q.num);
                if (costs > 0) {
                    ordersys.DelOrder(q);
                    q.status = OrderStatus::kSUCCESS;
                    ordersys.AddOrder(q);
                }
            }
        }
        bool st = ordersys.Refund(order);
        std::cout << (st ? 0 : -1) << "\n";
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
            std::cout << times << " ";
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
                BuyTicket(timestamp);
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