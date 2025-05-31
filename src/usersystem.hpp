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
    int privilege;
    bool loggined = 0;

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
    BPlusTree<string20, User> users_{"users"};
    BPlusTree<bool, string20> loggined_{"loggined"};

public:
    bool AddUser(const User &user) {
        if (users_.Find(user.username).size()) {
            return false;
        }
        // std::cerr << "adduser: " << user.username << "\n";
        users_.Insert(user.username, user);
        return 1;
    }
    void Login(User user) {
        // std::cerr << "testlogin0: " << users_.Find(user.username).size() << "\n";
        users_.Remove(user.username, user);
        // std::cerr << "testlogin0: " << users_.Find(user.username).size() << "\n";
        user.loggined = 1;
        users_.Insert(user.username, user);
        // std::cerr << "testlogin1: " << user.username << " " << users_.Find(user.username).size() << "\n";
        // auto p = users_.Allvalues();
        // std::cerr << "allnode\n";
        // for (auto x : p) {
        //     std::cerr << x.username << " ";
        // }
        // std::cerr << "\n";
        loggined_.Insert(1, user.username);
    }
    void Logout(User user) {
        users_.Remove(user.username, user);
        user.loggined = 0;
        users_.Insert(user.username, user);
        loggined_.Remove(1, user.username);
    }
    pair<User, bool> QueryUser(const string20 &username) {
        auto ve = users_.Find(username);
        if (ve.empty()) {
            return {User(), 0};
        }
        return {ve[0], 1};
    }
    void ModifyPassword(User user, const string30 &new_password) {
        users_.Remove(user.username, user);
        user.password = new_password;
        users_.Insert(user.username, user);
    }
    void ModifyName(User user, const string15 &new_name) {
        users_.Remove(user.username, user);
        user.name = new_name;
        users_.Insert(user.username, user);
    }
    void ModifyMail(User user, const string30 &new_mail) {
        users_.Remove(user.username, user);
        user.mailaddr = new_mail;
        users_.Insert(user.username, user);
    }
    void ModifyPrivilege(User user, const int &new_privilege) {
        users_.Remove(user.username, user);
        user.privilege = new_privilege;
        users_.Insert(user.username, user);
    }
    void Clear() {
        users_.Clear();
        loggined_.Clear();
    }
};

#endif