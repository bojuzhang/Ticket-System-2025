#pragma once
#include <cassert>
#include <iostream>
#include <string>
#ifndef TRAINSYSTEM_HPP
#define TRAINSYSTEM_HPP

#include "bpt.hpp"
#include "mystl.hpp"
#include "usersystem.hpp"

using sjtu::MyArray;

struct Train {
    string20 trainid;
    int stationnum;
    MyArray<string30, 100> stations;
    int seatnum;
    MyArray<int, 100> prices;
    pair<int, int> starttime; // (hh, mm)
    MyArray<int, 100> traveltimes;
    MyArray<int, 100> stopovertimes;
    pair<pair<int, int>, pair<int, int>> saledates; // (begin, end); (mm, dd);
    char type;
    bool operator < (const Train &other) {
        return trainid < other.trainid;
    }
    bool operator > (const Train &other) {
        return trainid > other.trainid;
    }
    bool operator == (const Train &other) {
        return trainid == other.trainid;
    }
    bool operator <= (const Train &other) {
        return !((*this) > other);
    }
    bool operator >= (const Train &other) {
        return !((*this) < other);
    }
    bool operator != (const Train &other) {
        return !((*this) == other);
    }
};

class TrainSystem {
private:
    BPlusTree<string20, int> trainidx{"trainidx"};
    sjtu::MemoryRiver<Train> trains;
    BPlusTree<string20, bool> released{"released"};
    struct RemainSeat {
        int stationnum;
        MyArray<int, 100> seats;
    };
    using TrainInDay = pair<pair<int, int>, string20>; // date, id
    BPlusTree<TrainInDay, int> remainseatidx{"remainseatidx"};
    MemoryRiver<RemainSeat> remainseat;
    struct TrainTicket {
        string20 trainid;
        int addday;
        MyArray<int, 2> leaving, arriving;
        int deltaday;
        int time, cost;
        int startpos, endpos;
    };
    struct TrainTime : TrainTicket {
        bool operator < (const TrainTime &other) {
            return (time != other.time) ? time < other.time : trainid < other.trainid;
        }
        bool operator > (const TrainTime &other) {
            return (time != other.time) ? time > other.time : trainid > other.trainid;
        }
        bool operator == (const TrainTime &other) {
            return trainid == other.trainid;
        }
        bool operator <= (const TrainTime &other) {
            return !((*this) > other);
        }
        bool operator >= (const TrainTime &other) {
            return !((*this) < other);
        }
        bool operator != (const TrainTime &other) {
            return !((*this) == other);
        }
    };
    struct TrainCost : TrainTicket {
        bool operator < (const TrainCost &other) {
            return (cost != other.cost) ? cost < other.cost : trainid < other.trainid;
        }
        bool operator > (const TrainCost &other) {
            return (cost != other.cost) ? cost > other.cost : trainid > other.trainid;
        }
        bool operator == (const TrainCost &other) {
            return trainid == other.trainid;
        }
        bool operator <= (const TrainCost &other) {
            return !((*this) > other);
        }
        bool operator >= (const TrainCost &other) {
            return !((*this) < other);
        }
        bool operator != (const TrainCost &other) {
            return !((*this) == other);
        }
    };
    BPlusTree<pair<string30, string30>, TrainTime, 8, 8> traintime{"traintime"};
    BPlusTree<pair<string30, string30>, TrainCost, 8, 8> traincost{"traincost"};
    struct TransferInfo {
        TrainTicket ticket;
        string30 to;
        pair<pair<int, int>, pair<int, int>> saledates;
        int idx;
        bool operator < (const TransferInfo &other) {
            return idx < other.idx;
        }
        bool operator > (const TransferInfo &other) {
            return idx > other.idx;
        }
        bool operator == (const TransferInfo &other) {
            return idx == other.idx;
        }
        bool operator <= (const TransferInfo &other) {
            return idx <= other.idx;
        }
        bool operator >= (const TransferInfo &other) {
            return idx >= other.idx;
        }
        bool operator != (const TransferInfo &other) {
            return idx != other.idx;
        }
    };
    BPlusTree<string30, TransferInfo, 16, 16> stations{"stations"};
    int idxstations;
    BPlusTree<pair<string30, string30>, TransferInfo, 16, 16> transnext{"transnext"};
    int idxtransnext;

    pair<int, int> AddDay(pair<int, int> date, int x) {
        date.second += x;
        if (date.second > (date.first == 6 ? 30 : 31)) {
            date.second -= (date.first == 6 ? 30 : 31);
            ++date.first;
        }
        return date;
    }

public:
    bool Debug = 0;

    TrainSystem() {
        idxstations = stations.GetInfo();
        idxtransnext = transnext.GetInfo();
        trains.initialise("trains", 1);
        remainseat.initialise("remainseat", 1);
    }
    ~TrainSystem() {
        stations.AddInfo(idxstations);
        transnext.AddInfo(idxtransnext);
    }

    void Clear() {
        trains.clear();
        released.Clear();
        remainseat.clear();
        traintime.Clear();
        traincost.Clear();
        stations.Clear();
        transnext.Clear();
        trainidx.Clear();
        remainseatidx.Clear();
        idxstations = idxtransnext = 0;
    }
    bool AddTrain(const Train &train) {
        if (trainidx.Find(train.trainid).size()) {
            return false;
        }
        int idx = trains.write(const_cast<Train&>(train));
        trainidx.Insert(train.trainid, idx);
        return true;
    }
    bool DeleteTrain(const string20 &trainid) {
        auto p = trainidx.Find(trainid);
        if (!p.size()) {
            return false;
        }
        int idx = p[0];
        Train train;
        trains.read(train, idx);
        if (released.Find(trainid).size()) {
            return false;
        }
        trainidx.Remove(trainid, idx);
        return true;
    }
    bool ReleaseTrain(const string20 &trainid) {
        auto p = trainidx.Find(trainid);
        if (!p.size()) {
            return false;
        }
        int idx = p[0];
        Train train;
        trains.read(train, idx);
        if (released.Find(trainid).size()) {
            return false;
        }
        RemainSeat t;
        t.stationnum = train.stationnum;
        for (int i = 0; i < t.stationnum; i++) {
            t.seats[i] = train.seatnum;
        }
        for (int m = train.saledates.first.first; m <= train.saledates.second.first; m++) {
            int db = m == train.saledates.first.first ? train.saledates.first.second : 1;
            int de = m == train.saledates.second.first ? train.saledates.second.second : 
                        (m == 6 ? 30 : 31);
            // std::cerr << "test " << m << " " << db << " " << de << "\n";
            for (int d = db; d <= de; d++) {        
                int idx = remainseat.write(t);
                // std::cerr << idx << " " << train.trainid << "\n";
                remainseatidx.Insert({pair{m, d}, trainid}, idx);
            }
        }
        int sminutes = train.starttime.first * 60 + train.starttime.second;
        for (int i = 0; i + 1 < train.stationnum; i++) {
            int time = train.traveltimes[i];
            int cost = train.prices[i];
            MyArray<int, 2> lea, arr;
            lea[0] = sminutes % 1440 / 60, lea[1] = sminutes % 1440 % 60;
            int addday = sminutes / 1440;
            for (int j = i + 1; j < train.stationnum; j++) {
                int minutes = sminutes + time;
                arr[0] = minutes % 1440 / 60, arr[1] = minutes % 1440 % 60;
                TrainTicket ticket = {trainid, addday, lea, arr, minutes / 1440 - sminutes / 1440, time, cost, i, j};
                traintime.Insert(pair{train.stations[i], train.stations[j]}, {trainid, addday, lea, arr, minutes / 1440 - sminutes / 1440, time, cost, i, j});
                traincost.Insert(pair{train.stations[i], train.stations[j]}, {trainid, addday, lea, arr, minutes / 1440 - sminutes / 1440, time, cost, i, j});
                time += train.stopovertimes[j - 1] + train.traveltimes[j];
                cost += train.prices[j];
                TransferInfo trans;
                trans.ticket = ticket;
                trans.to = train.stations[j];
                trans.saledates = train.saledates;
                trans.idx = ++idxstations;
                stations.Insert(train.stations[i], trans);
                trans.idx = ++idxtransnext;
                transnext.Insert({train.stations[i], train.stations[j]}, trans);
            }
            sminutes += train.traveltimes[i];
            if (i + 1 < train.stationnum) sminutes += train.stopovertimes[i];
        }
        released.Insert(trainid, 1);
        return true;
    }
    struct TrainInfo {
        string30 station;
        MyArray<int, 4> arriving, leaving;
        int price, seat;
    };
    pair<vector<TrainInfo>, char> QueryTrain(const string20 &trainid, pair<int, int> date) {
        auto p = trainidx.Find(trainid);
        if (!p.size()) {
            return {};
        }
        int idx = p[0];
        Train train;
        trains.read(train, idx);
        int m = date.first, d = date.second;
        if (m < train.saledates.first.first || m > train.saledates.second.first
        || (m == train.saledates.first.first && d < train.saledates.first.second)
        || (m == train.saledates.second.first && d > train.saledates.second.second)) {
            return {};
        }
        vector<TrainInfo> ans;
        MyArray<int, 4> arr, lea;
        arr[0] = arr[1] = arr[2] = arr[3] = -1;
        lea[0] = m, lea[1] = d, lea[2] = train.starttime.first, lea[3] = train.starttime.second;
        ans.push_back({train.stations[0], arr, lea, 0, train.seatnum});
        int minutes = lea[2] * 60 + lea[3];
        int prices = 0;
        for (int i = 0; i + 2 < train.stationnum; i++) {
            minutes += train.traveltimes[i];
            if (minutes >= 1440) {
                minutes -= 1440;
                d++;
            }
            if (d > (m == 6 ? 30 : 31)) {
                d -= (m == 6 ? 30 : 31);
                m++;
            }
            arr[0] = m, arr[1] = d, arr[2] = minutes / 60, arr[3] = minutes % 60;
            minutes += train.stopovertimes[i];
            if (minutes >= 1440) {
                minutes -= 1440;
                d++;
            }
            if (d > (m == 6 ? 30 : 31)) {
                d -= (m == 6 ? 30 : 31);
                m++;
            }
            lea[0] = m, lea[1] = d, lea[2] = minutes / 60, lea[3] = minutes % 60;
            prices += train.prices[i];
            ans.push_back({train.stations[i + 1], arr, lea, prices, train.seatnum});
        }
        minutes += train.traveltimes[train.stationnum - 2];
        if (minutes >= 1440) {
            minutes -= 1440;
            d++;
        }
        if (d > (m == 6 ? 30 : 31)) {
            d -= (m == 6 ? 30 : 31);
            m++;
        }
        arr[0] = m, arr[1] = d, arr[2] = minutes / 60, arr[3] = minutes % 60;
        lea[0] = lea[1] = lea[2] = lea[3] = -1;
        prices += train.prices[train.stationnum - 2];
        ans.push_back({train.stations[train.stationnum - 1], arr, lea, prices, -1});
        if (released.Find(trainid).size()) {
            m = date.first, d = date.second;
            int idx = remainseatidx.Find(pair{pair{m, d}, trainid})[0];
            RemainSeat p;
            remainseat.read(p, idx);
            for (int i = 0; i + 1 < train.stationnum; i++) {
                ans[i].seat = p.seats[i];
            }
        }
        return {ans, train.type};
    }
    struct TicketInfo {
        string20 trainid;
        string30 from, to;
        MyArray<int, 4> arriving, leaving;
        int price, seat;
        TrainTicket ticketinfo;
    }; 
    enum class TicketOrder {kTIME, kCOST};
    pair<TicketInfo, bool> GetTicketInfo(const TrainTicket &p, int m, int d, const string30 &st, const string30 &ed) {
        MyArray<int, 4> lea, arr;
        TicketInfo t;
        t.trainid = p.trainid;
        t.from = st, t.to = ed;
        lea[0] = m, lea[1] = d, lea[2] = p.leaving[0], lea[3] = p.leaving[1];
        arr[0] = m, arr[1] = d + p.deltaday, arr[2] = p.arriving[0], arr[3] = p.arriving[1];
        if (arr[1] > (m == 6 ? 30 : 31)) {
            arr[1] -= (m == 6 ? 30 : 31);
            arr[0]++;
        }
        t.arriving = arr, t.leaving = lea;
        t.price = p.cost;
        int am = m, ad = d - p.addday;
        if (ad < 1) {
            ad += (m == 7 ? 30 : 31);
            am--;
        }
        // if (Debug) {
        //     std::cerr << "time: " << am << " " << ad << "\n";
        // }
        auto seatidxs = remainseatidx.Find(pair{pair{am, ad}, t.trainid});
        if (!seatidxs.size()) {
            return {TicketInfo(), 0};
        } 
        RemainSeat seat;
        remainseat.read(seat, seatidxs[0]);
        // if (Debug) {
        //     std::cerr << seatidxs[0] << "\n";
        // }
        t.seat = seat.seats[p.startpos];
        for (int k = p.startpos; k < p.endpos; k++) {
            // if (Debug) {
            //     std::cerr << k << " " << seat.seats[k] << "\n";
            // }
            t.seat = std::min(t.seat, seat.seats[k]);
        }
        t.ticketinfo = p;
        return {t, 1};
    }
    vector<TicketInfo> QueryTicket(const string30 &st, const string30 &ed, pair<int, int> date, TicketOrder ord = TrainSystem::TicketOrder::kTIME) {
        vector<TicketInfo> ans;
        int m = date.first, d = date.second;
        if (ord == TicketOrder::kTIME) {
            auto ve = traintime.Find(pair{st, ed});
            MyArray<int, 4> lea, arr;
            for (auto p : ve) {
                auto [t, has] = GetTicketInfo(p, m, d, st, ed);
                if (!has) {
                    continue;
                }
                ans.push_back(t);
            }
        } else {
            auto ve = traincost.Find(pair{st, ed});
            for (auto p : ve) {
                auto [t, has] = GetTicketInfo(p, m, d, st, ed);
                if (!has) {
                    continue;
                }
                ans.push_back(t);
            }
        }
        return ans;
    }
    struct OrderInfo {
        MyArray<int, 4> leaving, arriving;
        int price = 0;
    };
    // 0: no train; 1: no tickets; 2: normal
    pair<OrderInfo, int> BuyTickets(const string20 &trainid, pair<int, int> date, const string30 &st, const string30 &ed, int n) {
        auto p = trainidx.Find(trainid);
        if (!p.size()) {
            return {OrderInfo(), 0};
        }
        int idx = p[0];
        Train train;
        trains.read(train, idx);
        if (n > train.seatnum) {
            return {OrderInfo(), 0};
        }
        bool flag = 0;
        OrderInfo order;
        int m = date.first, d = date.second;
        MyArray<int, 4> lea, arr;
        int adddays = 0;
        arr[0] = arr[1] = arr[2] = arr[3] = -1;
        lea[0] = m, lea[1] = d, lea[2] = train.starttime.first, lea[3] = train.starttime.second;
        int minutes = lea[2] * 60 + lea[3];
        bool has_st = 0, has_ed = 0;
        if (train.stations[0] == st) {
            order.leaving = lea;
            flag = 1;
            has_st = 1;
        }
        for (int i = 0; i + 2 < train.stationnum; i++) {
            minutes += train.traveltimes[i];
            if (minutes >= 1440) {
                minutes -= 1440;
                d++;
                if (!flag) {
                    ++adddays;
                }
            }
            if (d > (m == 6 ? 30 : 31)) {
                d = 1;
                m++;
            }
            arr[0] = m, arr[1] = d, arr[2] = minutes / 60, arr[3] = minutes % 60;
            minutes += train.stopovertimes[i];
            if (minutes >= 1440) {
                minutes -= 1440;
                d++;
                if (!flag) {
                    ++adddays;
                }
            }
            if (d > (m == 6 ? 30 : 31)) {
                d = 1;
                m++;
            }
            lea[0] = m, lea[1] = d, lea[2] = minutes / 60, lea[3] = minutes % 60;
            if (train.stations[i + 1] == st) {
                order.leaving = lea;
                flag = 1;
                has_st = 1;
            } else if (train.stations[i + 1] == ed) {
                order.arriving = arr;
                has_ed = 1;
            }
        }
        minutes += train.traveltimes[train.stationnum - 2];
        if (minutes >= 1440) {
            minutes -= 1440;
            d++;
            if (!flag) {
                ++adddays;
            }
        }
        if (d > (m == 6 ? 30 : 31)) {
            d = 1;
            m++;
        }
        arr[0] = m, arr[1] = d, arr[2] = minutes / 60, arr[3] = minutes % 60;
        lea[0] = lea[1] = lea[2] = lea[3] = -1;
        if (train.stations[train.stationnum - 1] == ed) {
            order.arriving = arr;
            has_ed = 1;
        }
        if (!has_st || !has_ed) {
            return {OrderInfo(), 0};
        }
        if (Debug) {
            std::cerr << "date: " << date.first << " " << date.second << " " << adddays << "\n";
        }
        order.arriving[1] -= adddays;
        if (order.arriving[1] <= 0) {
            order.arriving[1] += (order.arriving[0] == 7 ? 30 : 31);
            order.arriving[0]--;
        }
        order.leaving[1] -= adddays;
        if (order.leaving[1] <= 0) {
            order.leaving[1] += (order.leaving[0] == 7 ? 30 : 31);
            order.leaving[0]--;
        }
        date.second -= adddays;
        if (date.second <= 0) {
            date.second += (date.first == 7 ? 30 : 31);
            date.first--;
        }
        if (Debug) {
            std::cerr << "date: " << date.first << " " << date.second << " " << adddays << "\n";
        }
        auto q = remainseatidx.Find(pair{date, trainid});
        if (!q.size()) {
            return {OrderInfo(), 0};
        }
        flag = 0;
        for (int i = 0; i < train.stationnum; i++) {
            if (train.stations[i] == st) {
                flag = 1;
            }
            if (!flag && train.stations[i] == ed) {
                return {OrderInfo(), 0};
            }
            if (train.stations[i] == ed) {
                flag = 0;
            }
            if (flag) {
                order.price += train.prices[i];
            }
        }
        RemainSeat seats;
        remainseat.read(seats, q[0]);
        flag = 0;
        for (int i = 0; i < train.stationnum; i++) {
            if (train.stations[i] == st) {
                flag = 1;
            }
            if (train.stations[i] == ed) {
                flag = 0;
            }
            if (flag) {
                if (seats.seats[i] < n) {
                    return {order, 1};
                }
            }
        }
        flag = 0;
        for (int i = 0; i < train.stationnum; i++) {
            if (train.stations[i] == st) {
                flag = 1;
            }
            if (train.stations[i] == ed) {
                flag = 0;
            }
            if (flag) {
                seats.seats[i] -= n;
            }
        }
        remainseat.update(seats, q[0]);
        return {order, 2};
    }
    struct TransferTicket {
        TicketInfo first;
        TicketInfo second;
        int deltatime;
    };
    bool TransferTime(const TransferTicket &a, const TransferTicket &b) {
        if (a.first.ticketinfo.time + a.second.ticketinfo.time + a.deltatime != b.first.ticketinfo.time + b.second.ticketinfo.time + b.deltatime) {
            return a.first.ticketinfo.time + a.second.ticketinfo.time + a.deltatime < b.first.ticketinfo.time + b.second.ticketinfo.time + b.deltatime;
        }
        if (a.first.ticketinfo.cost + a.second.ticketinfo.cost != b.first.ticketinfo.cost + b.second.ticketinfo.cost) {
            return a.first.ticketinfo.cost + a.second.ticketinfo.cost < b.first.ticketinfo.cost + b.second.ticketinfo.cost;
        }
        if (a.first.trainid != b.first.trainid) {
            return a.first.trainid < b.first.trainid;
        }
        return a.second.trainid < b.second.trainid;
    }
    bool TransferCost(const TransferTicket &a, const TransferTicket &b) {
        if (a.first.ticketinfo.cost + a.second.ticketinfo.cost != b.first.ticketinfo.cost + b.second.ticketinfo.cost) {
            return a.first.ticketinfo.cost + a.second.ticketinfo.cost < b.first.ticketinfo.cost + b.second.ticketinfo.cost;
        }
        if (a.first.ticketinfo.time + a.second.ticketinfo.time + a.deltatime != b.first.ticketinfo.time + b.second.ticketinfo.time + b.deltatime) {
            return a.first.ticketinfo.time + a.second.ticketinfo.time + a.deltatime < b.first.ticketinfo.time + b.second.ticketinfo.time + b.deltatime;
        }
        if (a.first.trainid != b.first.trainid) {
            return a.first.trainid < b.first.trainid;
        }
        return a.second.trainid < b.second.trainid;
    }
    pair<TransferTicket, bool> QueryTransfer(const string30 &st, const string30 &ed, pair<int, int> date, TicketOrder ord = TrainSystem::TicketOrder::kTIME) {
        auto get_delta = [&](MyArray<int, 4> a, MyArray<int, 4> b) {
            int res = 0;
            if (a[0] == b[0]) {
                res = (b[1] - a[1]) * 24 * 60;
            } else if (a[0] == 6 && b[0] == 7) {
                res = (b[1] + 30 - a[1]) * 24 * 60;
            } else if (a[0] == 6 && b[0] == 8) {
                res = (b[1] + 30 + 31 - a[1]) * 24 * 60;
            } else if (a[0] == 7 && b[0] == 8) {
                res = (b[1] + 31 - a[1]) * 24 * 60;
            } else {
                return -1;
            }
            res += (b[2] * 60 + b[3]) - (a[2] * 60 + a[3]);
            return res;
        };
        TransferTicket ans;
        bool has_ans = 0;
        auto transsstaions = stations.Find(st);
        for (auto p1 : transsstaions) {
            auto &trans = p1.to;
            auto [t1, has1] = GetTicketInfo(p1.ticket, date.first, date.second, st, trans);
            if (!has1) {
                continue;
            }
            auto q = transnext.Find({trans, ed});
            if (!q.size()) {
                continue;
            }
            // auto ToString2 = [&](int x) -> std::string {
            //     if (x == -1) {
            //         return "xx";
            //     }
            //     std::string s = std::to_string(x);
            //     if (x < 10) {
            //         s = "0" + s;
            //     }
            //     return s;
            // };
            // if (Debug) {
            //     std::cerr << t1.trainid << " ";
            //     std::cerr << t1.from << " ";
            //     std::cerr << ToString2(t1.leaving[0]) << "-" << ToString2(t1.leaving[1]) << " ";
            //     std::cerr << ToString2(t1.leaving[2]) << ":" << ToString2(t1.leaving[3]) << " -> ";
            //     std::cerr << t1.to << " ";
            //     std::cerr << ToString2(t1.arriving[0]) << "-" << ToString2(t1.arriving[1]) << " ";
            //     std::cerr << ToString2(t1.arriving[2]) << ":" << ToString2(t1.arriving[3]) << " ";
            //     std::cerr << t1.price << " " << t1.seat << "\n";
            // }
            // std::cerr << "test " << st << " " << trans << "\n";
            pair<int, int> todate = {t1.arriving[0], t1.arriving[1]};
            for (auto p2 : q) {
                if (p2.ticket.trainid == t1.trainid) {
                    continue;
                }
                pair<int, int> realdatel = p2.saledates.first, realdater = p2.saledates.second;
                realdatel = AddDay(realdatel, p2.ticket.addday);
                realdater = AddDay(realdater, p2.ticket.addday);
                // std::cerr << realdatel.first << " " << realdatel.second << "\n";
                // std::cerr << realdater.first << " " << realdater.second << "\n";
                if (todate > realdater) {
                    continue;
                }
                pair<int, int> transferdate;
                if (todate < realdatel) {
                    transferdate = realdatel;
                } else if (pair{t1.arriving[2], t1.arriving[3]} > pair{p2.ticket.leaving[0], p2.ticket.leaving[1]}) {
                    transferdate = AddDay(todate, 1);
                } else {
                    transferdate = todate;
                }
                auto [t2, has2] = GetTicketInfo(p2.ticket, transferdate.first, transferdate.second, trans, ed);
                if (!has2) {
                    continue;
                }
                TransferTicket res = {t1, t2, get_delta(t1.arriving, t2.leaving)};
                if (!has_ans || (ord == TicketOrder::kTIME && TransferTime(res, ans)) || (ord == TicketOrder::kCOST && TransferCost(res, ans))) {
                    has_ans = 1;
                    ans = res;
                }
            }
        }
        return {ans, has_ans};
    }
};

#endif