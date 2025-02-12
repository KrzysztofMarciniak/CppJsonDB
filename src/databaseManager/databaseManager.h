#ifndef DATABASE_MANAGER_H
#define DATABASE_MANAGER_H

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>
#include <set>
using json = nlohmann::json;


enum class Permission {
    READ,
    WRITE,
    ADD_TO_TABLE,
    MANAGE_PERMISSIONS
};

struct Database {
    std::string owner;
    std::unordered_map<std::string, std::unordered_set<Permission>> permissions;
};

class DatabaseManager {
private:
    static const std::string DATABASE_PATH;
    std::unordered_map<std::string, Database> databases;
    DatabaseManager();
    void databaseNotFound(const std::string& dbName) const;
    bool addUserPermission(json& dbJson, const std::string& username, Permission permission);
    void appendDatabaseToJson(const std::string& name, const std::string& owner, json& jsonData);
    bool writeJsonToFile(const std::string& filePath, const json& jsonData);
public:
    std::string permissionToString(Permission permission) const;
    static Permission stringToPermission(const std::string& permString);

    bool readJsonFromFile(const std::string& filePath, json& jsonData) const;
    bool isOwner(const std::string& dbName, const std::string& username) const;
    static DatabaseManager& getInstance();
    bool deleteDatabase(const std::string& name);
    void listDatabases() const;
    bool canManagePermissions(const std::string& dbName, const std::string& username) const;
    bool canUseDatabase(const std::string& dbName, const std::string& username) const;
    void loadDatabasesFromFile();
    bool createDatabase(const std::string& name, const std::string& owner);
    bool removeUserPermission(const std::string& dbName, const std::string& username, const std::string& permission);
    bool checkUserPermission(const std::string& dbName, const std::string& username, Permission permission) const;
    std::unordered_set<Permission> getUserPermissions(const std::string& dbName, const std::string& username) const;
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_set<Permission>>> getDatabasePermissions(const std::string& dbName) const;
    bool databaseExists(const std::string& dbName) const;
    bool addPermission(const std::string& dbName, const std::string& username, Permission permission);

    json::iterator findDatabase(json& databases, const std::string& dbName) const;
    json::iterator findUser(json& users, const std::string& username) const;
    bool removePermissionFromUser(json& userJson, const std::string& permission);
};
#define INIT_DB_MANAGER auto& dbManager = DatabaseManager::getInstance();
#define IS_OWNER(dbName, username) dbManager.isOwner(dbName, username);
#define CHECK_USER_PERMISSION(dbName, username, permission) dbManager.checkUserPermission(dbName, username, permission);

#endif
