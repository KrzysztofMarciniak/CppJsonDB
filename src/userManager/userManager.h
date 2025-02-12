#ifndef USER_MANAGER_H
#define USER_MANAGER_H

#include "user.h"
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <unistd.h>
#include <nlohmann/json.hpp>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>
#include <sys/stat.h>  // mkdir
using json = nlohmann::json;


constexpr const char* USERS_FILE = "data/users.json";

class UserManager {
public:
    static UserManager& getInstance();
    UserManager(const UserManager&) = delete;
    UserManager& operator=(const UserManager&) = delete;
    bool createUser(const std::string& username, const std::string& password);
    bool login(const std::string& username, const std::string& password);
private:
    UserManager();
    std::unordered_map<std::string, User> users;
    std::string usersFile = USERS_FILE;
    bool loadFromFile();
    void saveToFile();
    std::string hashPassword(const std::string& password);
    bool checkPassword(const std::string& storedEncryptedPassword, const std::string& password);
};
#define INIT_USER_MANAGER auto& userManager = UserManager::getInstance();
#endif
