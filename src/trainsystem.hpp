#pragma once
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
    BPlusTree<string20, Train, 64> trains{"trains"};
    BPlusTree<string20, bool> released{"released"};
    struct RemainSeat {
        int stationnum;
        MyArray<int, 100> seats;
        bool operator < (const RemainSeat &other) {
            return stationnum < other.stationnum;
        }
        bool operator > (const RemainSeat &other) {
            return stationnum > other.stationnum;
        }
        bool operator == (const RemainSeat &other) {
            return stationnum == other.stationnum;
        }
        bool operator <= (const RemainSeat &other) {
            return !((*this) > other);
        }
        bool operator >= (const RemainSeat &other) {
            return !((*this) < other);
        }
        bool operator != (const RemainSeat &other) {
            return !((*this) == other);
        }
    };
    using TrainInDay = pair<pair<int, int>, string20>; // date, id
    BPlusTree<TrainInDay, RemainSeat, 32> remainseat{"remainseat"};
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
    BPlusTree<pair<string30, string30>, TrainTime> traintime{"traintime"};
    BPlusTree<pair<string30, string30>, TrainCost> traincost{"traincost"};
    BPlusTree<bool, string30> stations{"stations"};

public:
    bool Debug = 0;

    void Clear() {
        trains.Clear();
        released.Clear();
        remainseat.Clear();
        traintime.Clear();
        traincost.Clear();
        stations.Clear();
    }
    bool AddTrain(const Train &train) {
        if (trains.Find(train.trainid).size()) {
            return false;
        }
        trains.Insert(train.trainid, train);
        return true;
    }
    bool DeleteTrain(const string20 &trainid) {
        auto p = trains.Find(trainid);
        if (!p.size()) {
            return false;
        }
        Train train = p[0];
        if (released.Find(trainid).size()) {
            return false;
        }
        trains.Remove(trainid, train);
        return true;
    }
    bool ReleaseTrain(const string20 &trainid) {
        auto p = trains.Find(trainid);
        if (!p.size()) {
            return false;
        }
        Train train = p[0];
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
                remainseat.Insert({pair{m, d}, trainid}, t);
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
                traintime.Insert(pair{train.stations[i], train.stations[j]}, {trainid, addday, lea, arr, minutes / 1440 - sminutes / 1440, time, cost, i, j});
                traincost.Insert(pair{train.stations[i], train.stations[j]}, {trainid, addday, lea, arr, minutes / 1440 - sminutes / 1440, time, cost, i, j});
                time += train.stopovertimes[j - 1] + train.traveltimes[j];
                cost += train.prices[j];
            }
            sminutes += train.traveltimes[i];
            if (i + 1 < train.stationnum) sminutes += train.stopovertimes[i];
        }
        for (int i = 0; i < train.stationnum; i++) {
            stations.Insert(1, train.stations[i]);
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
        auto p = trains.Find(trainid);
        if (!p.size()) {
            return {};
        }
        Train train = p[0];
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
            if (minutes > 1440) {
                minutes -= 1440;
                d++;
            }
            if (d > (m == 6 ? 30 : 31)) {
                d = 1;
                m++;
            }
            arr[0] = m, arr[1] = d, arr[2] = minutes / 60, arr[3] = minutes % 60;
            minutes += train.stopovertimes[i];
            if (minutes > 1440) {
                minutes -= 1440;
                d++;
            }
            if (d > (m == 6 ? 30 : 31)) {
                d = 1;
                m++;
            }
            lea[0] = m, lea[1] = d, lea[2] = minutes / 60, lea[3] = minutes % 60;
            prices += train.prices[i];
            ans.push_back({train.stations[i + 1], arr, lea, prices, train.seatnum});
        }
        minutes += train.traveltimes[train.stationnum - 2];
        if (minutes > 1440) {
            minutes -= 1440;
            d++;
        }
        if (d > (m == 6 ? 30 : 31)) {
            d = 1;
            m++;
        }
        arr[0] = m, arr[1] = d, arr[2] = minutes / 60, arr[3] = minutes % 60;
        lea[0] = lea[1] = lea[2] = lea[3] = -1;
        prices += train.prices[train.stationnum - 2];
        ans.push_back({train.stations[train.stationnum - 1], arr, lea, prices, -1});
        if (released.Find(trainid).size()) {
            m = date.first, d = date.second;
            auto p = remainseat.Find(pair{pair{m, d}, trainid})[0];
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
    vector<TicketInfo> QueryTicket(const string30 &st, const string30 &ed, pair<int, int> date, TicketOrder ord = TrainSystem::TicketOrder::kCOST) {
        vector<TicketInfo> ans;
        int m = date.first, d = date.second;
        if (ord == TicketOrder::kTIME) {
            auto ve = traintime.Find(pair{st, ed});
            MyArray<int, 4> lea, arr;
            for (auto p : ve) {
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
                if (Debug) {
                    std::cerr << "time: " << am << " " << ad << "\n";
                }
                auto seat = remainseat.Find(pair{pair{am, ad}, t.trainid});
                if (!seat.size()) {
                    continue;
                } 
                t.seat = seat[0].seats[p.startpos];
                for (int k = p.startpos; k < p.endpos; k++) {
                    if (Debug) {
                        std::cerr << k << " " << seat[0].seats[k] << "\n";
                    }
                    t.seat = std::min(t.seat, seat[0].seats[k]);
                }
                t.ticketinfo = p;
                ans.push_back(t);
            }
        } else {
            auto ve = traincost.Find(pair{st, ed});
            MyArray<int, 4> lea, arr;
            for (auto p : ve) {
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
                if (Debug) {
                    std::cerr << am << " " << ad << "\n";
                }
                auto seat = remainseat.Find(pair{pair{am, ad}, t.trainid});
                if (!seat.size()) {
                    continue;
                } 
                t.seat = seat[0].seats[p.startpos];
                for (int k = p.startpos; k < p.endpos; k++) {
                    if (Debug) {
                        std::cerr << k << " " << t.seat << " " << seat[0].seats[k] << "\n";
                    }
                    t.seat = std::min(t.seat, seat[0].seats[k]);
                }
                t.ticketinfo = p;
                ans.push_back(t);
            }
        }
        return ans;
    }
    struct OrderInfo {
        MyArray<int, 4> leaving, arriving;
        int price = 0;
    };
    pair<OrderInfo, bool> BuyTickets(const string20 &trainid, pair<int, int> date, const string30 &st, const string30 &ed, int n) {
        auto p = trains.Find(trainid);
        if (!p.size()) {
            return {OrderInfo(), 0};
        }
        auto train = p[0];
        bool flag = 0;
        OrderInfo order;
        int m = date.first, d = date.second;
        MyArray<int, 4> lea, arr;
        int adddays = 0;
        arr[0] = arr[1] = arr[2] = arr[3] = -1;
        lea[0] = m, lea[1] = d, lea[2] = train.starttime.first, lea[3] = train.starttime.second;
        int minutes = lea[2] * 60 + lea[3];
        if (train.stations[0] == st) {
            order.leaving = lea;
            flag = 1;
        }
        for (int i = 0; i + 2 < train.stationnum; i++) {
            minutes += train.traveltimes[i];
            if (minutes > 1440) {
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
            if (minutes > 1440) {
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
            } else if (train.stations[i + 1] == ed) {
                order.arriving = arr;
            }
        }
        minutes += train.traveltimes[train.stationnum - 2];
        if (minutes > 1440) {
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
        auto q = remainseat.Find(pair{date, trainid});
        if (!q.size()) {
            return {OrderInfo(), 0};
        }
        auto seats = q[0];
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
        remainseat.Remove(pair{date, trainid}, seats);
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
                order.price += train.prices[i];
            }
        }
        remainseat.Insert(pair{date, trainid}, seats);
        return {order, 1};
    }
    struct TransferTicket {
        TicketInfo first;
        TicketInfo second;
    };
    bool TranseferTime(const TransferTicket &a, const TransferTicket &b) {
        if (a.first.ticketinfo.time + a.second.ticketinfo.time != b.first.ticketinfo.time + b.second.ticketinfo.time) {
            return a.first.ticketinfo.time + a.second.ticketinfo.time < b.first.ticketinfo.time + b.second.ticketinfo.time;
        }
        if (a.first.ticketinfo.cost + a.second.ticketinfo.cost != b.first.ticketinfo.cost + b.second.ticketinfo.cost) {
            return a.first.ticketinfo.cost + a.second.ticketinfo.cost < b.first.ticketinfo.cost + b.second.ticketinfo.cost;
        }
        if (a.first.trainid != b.first.trainid) {
            return a.first.trainid < b.first.trainid;
        }
        return a.second.trainid < b.second.trainid;
    }
    bool TranseferCost(const TransferTicket &a, const TransferTicket &b) {
        if (a.first.ticketinfo.cost + a.second.ticketinfo.cost != b.first.ticketinfo.cost + b.second.ticketinfo.cost) {
            return a.first.ticketinfo.cost + a.second.ticketinfo.cost < b.first.ticketinfo.cost + b.second.ticketinfo.cost;
        }
        if (a.first.ticketinfo.time + a.second.ticketinfo.time != b.first.ticketinfo.time + b.second.ticketinfo.time) {
            return a.first.ticketinfo.time + a.second.ticketinfo.time < b.first.ticketinfo.time + b.second.ticketinfo.time;
        }
        if (a.first.trainid != b.first.trainid) {
            return a.first.trainid < b.first.trainid;
        }
        return a.second.trainid < b.second.trainid;
    }
    pair<TransferTicket, bool> QueryTransfer(const string30 &st, const string30 &ed, pair<int, int> date, TicketOrder ord = TrainSystem::TicketOrder::kCOST) {
        auto allstation = stations.Find(1);
        TransferTicket ans;
        bool flag = 0;
        for (auto trans : allstation) {
            auto p1 = QueryTicket(st, trans, date, ord);
            auto p2 = QueryTicket(trans, ed, date, ord);
            if (!p1.size() || !p2.size()) {
                continue;
            }
            TransferTicket res = {p1[0], p2[0]};
            if (!flag || (ord == TicketOrder::kCOST && TranseferCost(res, ans))
            || (ord == TicketOrder::kTIME && TranseferTime(res, ans))) {
                ans = res;
                flag = 1;    
            }
        }
        return {ans, flag};
    }
};

#endif