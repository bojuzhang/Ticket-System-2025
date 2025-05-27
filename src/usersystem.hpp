#pragma once
#ifndef USERSYSTEM_HPP
#define USERSYSTEM_HPP

#include "bpt.hpp"
#include "mystl.hpp"

using string10 = sjtu::MyString<10>;
using string20 = sjtu::MyString<20>;
using string30 = sjtu::MyString<30>;

struct User {
    string20 username;
    string30 password;
    string10 name;
    string30 mailaddr;
    int privilege;
    bool loggined;
};

class UserSystem {
private:
    BPlusTree<string20, User> users_;
    BPlusTree<bool, string20> loggined_;

public:
    bool AddUser(const User &user) {
        if (users_.Find(user.username).size()) {
            return false;
        }
        users_.Insert(user.username, user);
        return 1;
    }
    void Login(User user) {
        users_.Remove(user.username, user);
        user.loggined = 1;
        users_.Insert(user.username, user);
        loggined_.Insert(1, user.username);
    }
    void Logout(User user) {
        users_.Remove(user.username, user);
        user.loggined = 0;
        users_.Insert(user.username, user);
        loggined_.Remove(1, user.username);
    }
    User QueryUser(const string20 &username) {
        return users_.Find(username)[0];
    }
    void ModifyPassword(User user, const string30 &new_password) {
        users_.Remove(user.username, user);
        user.password = new_password;
        users_.Insert(user.username, user);
    }
    void ModifyName(User user, const string10 &new_name) {
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
};

#endif