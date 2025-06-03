#pragma once
#include <iostream>
#ifndef USERSYSTEM_HPP
#define USERSYSTEM_HPP

#include "bpt.hpp"
#include "mystl.hpp"

using string15 = sjtu::MyString<15>;
using string20 = sjtu::MyString<20>;
using string30 = sjtu::MyString<30>;

struct User {
    string20 username;
    string30 password;
    string15 name;
    string30 mailaddr;
    short privilege;
    bool loggined = 0;
    // int idx;

    bool operator < (const User &other) {
        return username < other.username;
    }
    bool operator > (const User &other) {
        return username > other.username;
    }
    bool operator == (const User &other) {
        return username == other.username;
    }
    bool operator <= (const User &other) {
        return !((*this) > other);
    }
    bool operator >= (const User &other) {
        return !((*this) < other);
    }
    bool operator != (const User &other) {
        return !((*this) == other);
    }
};

class UserSystem {
private:
    BPlusTree<ull, short> useridx{"useridx"};
    MemoryRiver<User> users;
    BPlusTree<bool, string20> loggined{"loggined"};

public:
    UserSystem() {
        users.initialise("users", 1);
    }
    ~UserSystem() {
        auto logins = loggined.Find(1);
        for (auto p : logins) {
            auto tmp = QueryUser(p);
            Logout(tmp.first, tmp.second);
        }
    }

    bool Empty() {
        return users.size() == 0;
    }

    bool AddUser(const User &user) {
        if (useridx.Find(hash(user.username)).size()) {
            return false;
        }
        int idx = users.write(const_cast<User&>(user));
        useridx.Insert(hash(user.username), idx);
        return true;
    }
    void Login(User user, int idx) {
        user.loggined = 1;
        users.update(user, idx);
        loggined.Insert(1, user.username);
    }
    void Logout(User user, int idx) {
        user.loggined = 0;
        users.update(user, idx);
        loggined.Remove(1, user.username);
    }
    pair<User, int> QueryUser(const string20 &username) {
        auto ve = useridx.Find(hash(username));
        if (ve.empty()) {
            return {User(), -1};
        }
        User ans;
        users.read(ans, ve[0]);
        return {ans, ve[0]};
    }
    void Modify(const User &user, int idx) {
        users.update(const_cast<User&>(user), idx);
    }
    void Clear() {
        users.clear();
        loggined.Clear();
        useridx.Clear();
    }
};

#endif