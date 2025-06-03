#pragma once
#include <cassert>
#include <climits>
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
    short stationnum;
    MyArray<string30, 100> stations;
    int seatnum;
    MyArray<int, 100> prices;
    pair<short, short> starttime; // (hh, mm)
    MyArray<short, 100> traveltimes;
    MyArray<short, 100> stopovertimes;
    pair<pair<short, short>, pair<short, short>> saledates; // (begin, end); (mm, dd);
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
    BPlusTree<ull, short> trainidx{"trainidx"};
    sjtu::MemoryRiver<Train> trains;
    BPlusTree<ull, bool> released{"released"};
    struct RemainSeat {
        short stationnum;
        MyArray<int, 100> seats;
    };
    using TrainInDay = pair<pair<short, short>, ull>; // date, id
    BPlusTree<TrainInDay, int> remainseatidx{"remainseatidx"};
    MemoryRiver<RemainSeat> remainseat;
    struct TrainTicket {
        string20 trainid;
        short addday;
        MyArray<short, 2> leaving, arriving;
        short deltaday;
        short time;
        int cost;
    };
    MemoryRiver<TrainTicket> ticketidx;
    BPlusTree<pair<ull, ull>, int, 4, 8> trainticket{"trainticket"};
    struct TransferInfo {
        int ticketidx;
        pair<pair<short, short>, pair<short, short>> saledates;
        bool operator < (const TransferInfo &other) {
            return ticketidx < other.ticketidx;
        }
        bool operator > (const TransferInfo &other) {
            return ticketidx > other.ticketidx;
        }
        bool operator == (const TransferInfo &other) {
            return ticketidx == other.ticketidx;
        }
        bool operator <= (const TransferInfo &other) {
            return ticketidx <= other.ticketidx;
        }
        bool operator >= (const TransferInfo &other) {
            return ticketidx >= other.ticketidx;
        }
        bool operator != (const TransferInfo &other) {
            return ticketidx != other.ticketidx;
        }
    };
    BPlusTree<ull, string30> stations{"stations"};
    BPlusTree<pair<ull, ull>, TransferInfo> transnext{"transnext"};

    pair<short, short> AddDay(pair<short, short> date, int x) {
        date.second += x;
        if (date.second > (date.first == 6 ? 30 : 31)) {
            date.second -= (date.first == 6 ? 30 : 31);
            ++date.first;
        }
        return date;
    }
    int get_delta(MyArray<short, 4> a, MyArray<short, 4> b) {
        int res = 0;
        if (a[0] == b[0]) {
            res = (b[1] - a[1]) * 24 * 60;
        } else if (a[0] == 6 && b[0] == 7) {
            res = (b[1] + 30 - a[1]) * 24 * 60;
        } else if (a[0] == 6 && b[0] == 8) {
            res = (b[1] + 30 + 31 - a[1]) * 24 * 60;
        } else if (a[0] == 6 && b[0] == 9) {
            res = (b[1] + 31 + 31 + 30 - a[1]) * 24 * 60;
        } else if (a[0] == 7 && b[0] == 8) {
            res = (b[1] + 31 - a[1]) * 24 * 60;
        } else if (a[0] == 7 && b[0] == 9) {
            res = (b[1] + 31 + 31 - a[1]) * 24 * 60;
        } else if (a[0] == 8 && b[0] == 9) {
            res = (b[1] + 31 - a[1]) * 24 * 60;
        } else {
            return -1;
        }
        res += (b[2] * 60 + b[3]) - (a[2] * 60 + a[3]);
        return res;
    };

public:
    bool Debug = 0;

    TrainSystem() {
        trains.initialise("trains", 1);
        remainseat.initialise("remainseat", 1);
        ticketidx.initialise("ticketidx", 1);
    }

    void Clear() {
        trains.clear();
        released.Clear();
        remainseat.clear();
        trainticket.Clear();
        stations.Clear();
        transnext.Clear();
        trainidx.Clear();
        remainseatidx.Clear();
        ticketidx.clear();
    }
    bool AddTrain(const Train &train) {
        if (trainidx.Find(hash(train.trainid)).size()) {
            return false;
        }
        int idx = trains.write(const_cast<Train&>(train));
        trainidx.Insert(hash(train.trainid), idx);
        return true;
    }
    bool DeleteTrain(const string20 &trainid) {
        auto p = trainidx.Find(hash(trainid));
        if (!p.size()) {
            return false;
        }
        int idx = p[0];
        Train train;
        trains.read(train, idx);
        if (released.Find(hash(trainid)).size()) {
            return false;
        }
        trainidx.Remove(hash(trainid), idx);
        return true;
    }
    bool ReleaseTrain(const string20 &trainid) {
        auto p = trainidx.Find(hash(trainid));
        if (!p.size()) {
            return false;
        }
        int idx = p[0];
        Train train;
        trains.read(train, idx);
        if (released.Find(hash(trainid)).size()) {
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
                remainseatidx.Insert({pair{m, d}, hash(trainid)}, idx);
            }
        }
        int sminutes = train.starttime.first * 60 + train.starttime.second;
        for (int i = 0; i + 1 < train.stationnum; i++) {
            int time = train.traveltimes[i];
            int cost = train.prices[i];
            MyArray<short, 2> lea, arr;
            lea[0] = sminutes % 1440 / 60, lea[1] = sminutes % 1440 % 60;
            int addday = sminutes / 1440;
            for (int j = i + 1; j < train.stationnum; j++) {
                int minutes = sminutes + time;
                arr[0] = minutes % 1440 / 60, arr[1] = minutes % 1440 % 60;
                TrainTicket ticket = {trainid, (short)addday, lea, arr, (short)(minutes / 1440 - sminutes / 1440) ,(short)time, cost};
                int idx = ticketidx.write(ticket);
                trainticket.Insert(pair{hash(train.stations[i]), hash(train.stations[j])}, idx);
                time += train.stopovertimes[j - 1] + train.traveltimes[j];
                cost += train.prices[j];
                TransferInfo trans;
                trans.ticketidx = idx;
                trans.saledates = train.saledates;
                transnext.Insert({hash(train.stations[i]), hash(train.stations[j])}, trans);
                stations.Insert(hash(train.stations[i]), train.stations[j]);
            }
            sminutes += train.traveltimes[i];
            if (i + 1 < train.stationnum) sminutes += train.stopovertimes[i];
        }
        released.Insert(hash(trainid), 1);
        return true;
    }
    struct TrainInfo {
        string30 station;
        MyArray<short, 4> arriving, leaving;
        int price, seat;
    };
    pair<vector<TrainInfo>, char> QueryTrain(const string20 &trainid, pair<short, short> date) {
        auto p = trainidx.Find(hash(trainid));
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
        MyArray<short, 4> arr, lea;
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
        if (released.Find(hash(trainid)).size()) {
            m = date.first, d = date.second;
            int idx = remainseatidx.Find(pair{pair{m, d}, hash(trainid)})[0];
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
        MyArray<short, 4> arriving, leaving;
        int price, seat;
        TrainTicket ticketinfo;
    }; 
    enum class TicketOrder {kTIME, kCOST};
    pair<TicketInfo, bool> GetTicketInfo(const TrainTicket &p, int m, int d, const string30 &st, const string30 &ed) {
        MyArray<short, 4> lea, arr;
        TicketInfo t;
        auto ve = trainidx.Find(hash(p.trainid));
        int idx = ve[0];
        Train train;
        trains.read(train, idx);
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
        auto seatidxs = remainseatidx.Find(pair{pair{am, ad}, hash(t.trainid)});
        if (!seatidxs.size()) {
            return {TicketInfo(), 0};
        } 
        RemainSeat seat;
        remainseat.read(seat, seatidxs[0]);
        // if (Debug) {
        //     std::cerr << seatidxs[0] << "\n";
        // }
        t.seat = train.seatnum;
        bool flag = 0;
        for (int k = 0; k < seat.stationnum; k++) {
            // if (Debug) {
            //     std::cerr << k << " " << seat.seats[k] << "\n";
            // }
            if (st == train.stations[k]) {
                flag = 1;
            }
            if (ed == train.stations[k]) {
                flag = 0;
            }
            if (flag) {
                t.seat = std::min(t.seat, seat.seats[k]);
            }
        }
        t.ticketinfo = p;
        return {t, 1};
    }
    vector<TicketInfo> QueryTicket(const string30 &st, const string30 &ed, pair<short, short> date, TicketOrder ord = TrainSystem::TicketOrder::kTIME) {
        vector<TicketInfo> ans;
        int m = date.first, d = date.second;
        auto ve = trainticket.Find(pair{hash(st), hash(ed)});
        MyArray<short, 4> lea, arr;
        for (auto pos : ve) {
            TrainTicket p;
            ticketidx.read(p, pos);
            auto [t, has] = GetTicketInfo(p, m, d, st, ed);
            if (!has) {
                continue;
            }
            ans.push_back(t);
        }
        if (ord == TicketOrder::kTIME) {
            merge_sort(ans, [&](const auto &x, const auto &y) {
                int xtime = get_delta(x.leaving, x.arriving);
                int ytime = get_delta(y.leaving, y.arriving);
                if (xtime != ytime) {
                    return xtime < ytime;
                }
                return x.trainid < y.trainid;
            });
        } else {
            merge_sort(ans, [&](const auto &x, const auto &y) {
                if (x.price != y.price) {
                    return x.price < y.price;
                }
                return x.trainid < y.trainid;
            });
        }
        return ans;
    }
    struct OrderInfo {
        MyArray<short, 4> leaving, arriving;
        int price = 0;
    };
    // 0: no train; 1: no tickets; 2: normal
    pair<OrderInfo, int> BuyTickets(const string20 &trainid, pair<short, short> date, const string30 &st, const string30 &ed, int n) {
        auto p = trainidx.Find(hash(trainid));
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
        MyArray<short, 4> lea, arr;
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
        auto q = remainseatidx.Find(pair{date, hash(trainid)});
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
    pair<TransferTicket, bool> QueryTransfer(const string30 &st, const string30 &ed, pair<short, short> date, TicketOrder ord = TrainSystem::TicketOrder::kTIME) {
        TransferTicket ans;
        bool has_ans = 0;
        auto transstations = stations.Find(hash(st));
        for (auto trans : transstations) {
            auto ve = transnext.Find({hash(st), hash(trans)});
            // if (Debug) {
            //     std::cerr << trans << "\n";
            // }
            for (auto ti1 : ve) {
                int pos1 = ti1.ticketidx;
                TrainTicket p1;
                ticketidx.read(p1, pos1);
                // if (Debug) {
                //     std::cerr << "test " << pos1 << " " << p1.trainid << " " << trans << "\n";
                // }
                auto [t1, has1] = GetTicketInfo(p1, date.first, date.second, st, trans);
                if (!has1) {
                    continue;
                }
                // if (Debug) {
                //     std::cerr << "fhdj has1\n";
                // }
                auto q = transnext.Find({hash(trans), hash(ed)});
                if (!q.size()) {
                    continue;
                }
                pair<short, short> todate = {t1.arriving[0], t1.arriving[1]};
                for (auto ti2 : q) {
                    int pos2 = ti2.ticketidx;
                    TrainTicket p2;
                    ticketidx.read(p2, pos2);
                    if (p2.trainid == t1.trainid) {
                        continue;
                    }
                    pair<short, short> realdatel = ti2.saledates.first, realdater = ti2.saledates.second;
                    realdatel = AddDay(realdatel, p2.addday);
                    realdater = AddDay(realdater, p2.addday);
                    // std::cerr << realdatel.first << " " << realdatel.second << "\n";
                    // std::cerr << realdater.first << " " << realdater.second << "\n";
                    if (todate > realdater) {
                        continue;
                    }
                    pair<short, short> transferdate;
                    if (todate < realdatel) {
                        transferdate = realdatel;
                    } else if (pair{t1.arriving[2], t1.arriving[3]} > pair{p2.leaving[0], p2.leaving[1]}) {
                        transferdate = AddDay(todate, 1);
                    } else {
                        transferdate = todate;
                    }
                    auto [t2, has2] = GetTicketInfo(p2, transferdate.first, transferdate.second, trans, ed);
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
        }
        return {ans, has_ans};
    }
};

#endif