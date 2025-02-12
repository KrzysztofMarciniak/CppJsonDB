#include "databaseManager/databaseManager.h"
const std::string DatabaseManager::DATABASE_PATH = "data/databases.json";
DatabaseManager::DatabaseManager() {}

DatabaseManager& DatabaseManager::getInstance() {
    static DatabaseManager instance;
    return instance;
}

Permission DatabaseManager::stringToPermission(const std::string& permString) {
    if (permString == "READ") {
        return Permission::READ;
    } else if (permString == "WRITE") {
        return Permission::WRITE;
    } else if (permString == "ADD_TO_TABLE") {
        return Permission::ADD_TO_TABLE;
    } else if (permString == "MANAGE_PERMISSIONS") {
        return Permission::MANAGE_PERMISSIONS;
    } else {
        throw std::invalid_argument("Unknown permission string: " + permString);
    }
}

std::string DatabaseManager::permissionToString(Permission permission) const {
    switch (permission) {
        case Permission::READ:
            return "READ";
        case Permission::WRITE:
            return "WRITE";
        case Permission::ADD_TO_TABLE:
            return "ADD_TO_TABLE";
        case Permission::MANAGE_PERMISSIONS:
            return "MANAGE_PERMISSIONS";
        default:
            throw std::invalid_argument("Unknown permission value");
    }
}


void DatabaseManager::loadDatabasesFromFile() {
    std::ifstream inputFile(DATABASE_PATH);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Failed to open file 'data/databases.json' for reading." << std::endl;
        return;
    }

    json jsonData;
    inputFile >> jsonData;
    inputFile.close();

    databases.clear();
    for (const auto& dbJson : jsonData["databases"]) {
        std::string dbName = dbJson["name"];
        Database db;
        db.owner = dbJson["owner"];
        databases[dbName] = db;
    }
}
void DatabaseManager::listDatabases() const {
    std::cout << "Databases:\n";
    std::cout << "\n";
    std::cout << std::left << std::setw(20) << "Database Name" << "Owner\n";
    std::cout << std::string(40, '-') << '\n';

    for (const auto& dbPair : databases) {
        const std::string& dbName = dbPair.first;
        const std::string& dbOwner = dbPair.second.owner;
        std::cout << std::left << std::setw(20) << dbName << dbOwner << "\n";
    }
}

bool DatabaseManager::databaseExists(const std::string& dbName) const {
    std::ifstream inputFile(DATABASE_PATH);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Failed to open file 'data/databases.json'." << std::endl;
        return false;
    }

    json jsonData;
    try {
        inputFile >> jsonData;
    } catch (const std::exception& e) {
        std::cerr << "Error: Failed to parse JSON from file 'data/databases.json': " << e.what() << std::endl;
        inputFile.close();
        return false;
    }

    inputFile.close();

    if (!jsonData.contains("databases") || !jsonData["databases"].is_array()) {
        std::cerr << "Error: JSON file 'data/databases.json' does not contain 'databases' array." << std::endl;
        return false;
    }

    for (const auto& db : jsonData["databases"]) {
        std::string currentDbName = db["name"];
        if (currentDbName == dbName) {
            return true;
        }
    }

    std::cerr << "Database '" << dbName << "' not found in loaded databases." << std::endl;
    return false;
}

bool DatabaseManager::createDatabase(const std::string& name, const std::string& owner) {
    if (databases.find(name) != databases.end()) {
        std::cerr << "Error: Database '" << name << "' already exists." << std::endl;
        return false;
    }
    databases[name] = { owner };
    json jsonData;
    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        return false;
    }
    appendDatabaseToJson(name, owner, jsonData);
    if (!writeJsonToFile(DATABASE_PATH, jsonData)) {
        return false;
    }

    return true;
}


bool DatabaseManager::readJsonFromFile(const std::string& filePath, json& jsonData) const {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
        std::cerr << "Error: Failed to open file '" << filePath << "' for reading." << std::endl;
        return false;
    }

    try {
        inputFile >> jsonData;
    } catch (const std::exception& e) {
        std::cerr << "Error: Exception caught while reading JSON file: " << e.what() << std::endl;
        inputFile.close();
        return false;
    }

    inputFile.close();
    return true;
}


void DatabaseManager::appendDatabaseToJson(const std::string& name, const std::string& owner, json& jsonData) {
    if (jsonData.empty()) {
        jsonData["databases"] = json::array();
    }

    json dbJson;
    dbJson["name"] = name;
    dbJson["owner"] = owner;
    dbJson["users"] = json::array();

    jsonData["databases"].push_back(dbJson);
}

bool DatabaseManager::writeJsonToFile(const std::string& filePath, const json& jsonData) {
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open()) {
        std::cerr << "Error: Failed to open file '" << filePath << "' for writing." << std::endl;
        return false;
    }

    outputFile << std::setw(4) << jsonData << std::endl;
    outputFile.close();
    return true;
}
bool DatabaseManager::deleteDatabase(const std::string& name) {
    if (!databaseExists(name)) {
        std::cerr << "Error: Database '" << name << "' does not exist." << std::endl;
        return false;
    }

    databases.erase(name);
    json jsonData;
    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        return false;
    }
    json updatedJson;
    updatedJson["databases"] = json::array();
    for (const auto& dbJson : jsonData["databases"]) {
        if (dbJson["name"] != name) {
            updatedJson["databases"].push_back(dbJson);
        }
    }

    if (!writeJsonToFile(DATABASE_PATH, updatedJson)) {
        return false;
    }

    return true;
}

bool DatabaseManager::isOwner(const std::string& dbName, const std::string& username) const {
    json jsonData;
    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to read JSON file." << std::endl;
        return false;
    }
    for (const auto& dbJson : jsonData["databases"]) {
        if (dbJson["name"] == dbName) {
            std::string owner = dbJson["owner"];
            return owner == username;
        }
    }
    std::cerr << "Database '" << dbName << "' not found." << std::endl;
    return false;
}

json::iterator DatabaseManager::findDatabase(json& databases, const std::string& dbName) const {
    return std::find_if(databases.begin(), databases.end(), [&dbName](const json& dbJson) {
        return dbJson["name"] == dbName;
    });
}

json::iterator DatabaseManager::findUser(json& users, const std::string& username) const {
    return std::find_if(users.begin(), users.end(), [&username](const json& userJson) {
        return userJson["username"] == username;
    });
}

bool DatabaseManager::removePermissionFromUser(json& userJson, const std::string& permission) {
    if (!userJson.contains("permissions") || !userJson["permissions"].is_array()) {
        return false;
    }

    auto& permissionsArray = userJson["permissions"];
    permissionsArray.erase(
        std::remove_if(permissionsArray.begin(), permissionsArray.end(),
            [&permission](const json& permJson) {
                return permJson == permission;
            }),
        permissionsArray.end()
    );

    if (permissionsArray.empty()) {
        userJson.erase("permissions");
    }

    return true;
}
bool DatabaseManager::removeUserPermission(const std::string& dbName, const std::string& username, const std::string& permission) {
    json jsonData;

    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to read JSON file." << std::endl;
        return false;
    }

    auto databaseIt = findDatabase(jsonData["databases"], dbName);
    if (databaseIt == jsonData["databases"].end()) {
        std::cerr << "Database '" << dbName << "' not found." << std::endl;
        return false;
    }

    auto& dbJson = *databaseIt;
    if (!dbJson.contains("users") || !dbJson["users"].is_array()) {
        std::cerr << "Error: 'users' is not an array in database '" << dbName << "'." << std::endl;
        return false;
    }

    auto& usersArray = dbJson["users"];
    bool userFound = false;

    for (auto it = usersArray.begin(); it != usersArray.end(); ++it) {
        if ((*it)["username"] == username) {
            if (!it->contains("permissions") || !(*it)["permissions"].is_array()) {
                std::cerr << "Error: 'permissions' is not an array for user '" << username << "' in database '" << dbName << "'." << std::endl;
                return false;
            }

            auto& permissionsArray = (*it)["permissions"];
            permissionsArray.erase(
                std::remove_if(permissionsArray.begin(), permissionsArray.end(),
                    [&](const json& permJson) {
                        return permJson == permission;
                    }),
                permissionsArray.end()
            );

            if (permissionsArray.empty()) {
                usersArray.erase(it);
            }

            userFound = true;
            break;
        }
    }

    if (!userFound) {
        std::cerr << "User '" << username << "' not found in database '" << dbName << "'." << std::endl;
        return false;
    }

    if (!writeJsonToFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to update JSON file." << std::endl;
        return false;
    }

    std::cout << "Permission removed successfully." << std::endl;
    return true;
}

bool DatabaseManager::checkUserPermission(const std::string& dbName, const std::string& username, Permission permission) const {
    json jsonData;

    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to read JSON file." << std::endl;
        return false;
    }

    for (const auto& db : jsonData["databases"]) {
        if (db["name"] == dbName) {
            std::string dbOwner = db["owner"];
            if (dbOwner == username) {
                return true;
            }

            for (const auto& user : db["users"]) {
                if (user["username"] == username) {
                    auto& permissionsArray = user["permissions"];
                    for (const auto& perm : permissionsArray) {
                        if (perm == permissionToString(permission)) {
                            return true;
                        }
                    }
                    break;
                }
            }

            std::cerr << "User '" << username << "' does not have permission '" << permissionToString(permission) << "' for database '" << dbName << "'." << std::endl;
            return false;
        }
    }

    std::cerr << "Database '" << dbName << "' not found in JSON data." << std::endl;
    return false;
}


bool DatabaseManager::canManagePermissions(const std::string& dbName, const std::string& username) const {
    if (isOwner(dbName, username)) {
        return true;
    }
    return checkUserPermission(dbName, username, Permission::MANAGE_PERMISSIONS);
}
bool DatabaseManager::canUseDatabase(const std::string& dbName, const std::string& username) const {
    if (isOwner(dbName, username)) {
        return true;
    }
    for (auto perm : {Permission::READ, Permission::WRITE, Permission::ADD_TO_TABLE, Permission::MANAGE_PERMISSIONS}) {
        if (checkUserPermission(dbName, username, perm)) {
            return true;
        }
    }
    return false;
}

std::unordered_set<Permission> DatabaseManager::getUserPermissions(const std::string& dbName, const std::string& username) const {
    auto it = databases.find(dbName);
    if (it == databases.end()) {
        return {}; 
    }
    
    auto userIt = it->second.permissions.find(username);
    if (userIt == it->second.permissions.end()) {
        return {}; 
    }
    
    return userIt->second;
}
std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_set<Permission>>>
DatabaseManager::getDatabasePermissions(const std::string& dbName) const {
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_set<Permission>>> databasePermissions;

    json jsonData;
    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to read JSON file." << std::endl;
        return databasePermissions;
    }

    for (const auto& dbJson : jsonData["databases"]) {
        if (dbJson["name"] != dbName) {
            continue;
        }

        if (!dbJson.contains("users") || !dbJson["users"].is_array()) {
            break;
        }

        for (const auto& userJson : dbJson["users"]) {
            std::string username = userJson["username"];
            std::unordered_set<Permission> permissions;

            if (userJson.contains("permissions") && userJson["permissions"].is_array()) {
                for (const auto& perm : userJson["permissions"]) {
                    Permission permission = stringToPermission(perm);
                    permissions.insert(permission);
                }
            }

            databasePermissions[dbName][username] = permissions;
        }

        break;
    }

    return databasePermissions;
}

bool DatabaseManager::addPermission(const std::string& dbName, const std::string& username, Permission permission) {
    json jsonData;

    if (!readJsonFromFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to read JSON file." << std::endl;
        return false;
    }

    auto dbIt = findDatabase(jsonData["databases"], dbName);
    if (dbIt == jsonData["databases"].end()) {
        std::cerr << "Error: Database '" << dbName << "' not found in JSON data." << std::endl;
        return false;
    }

    if (!addUserPermission(*dbIt, username, permission)) {
        std::cerr << "Error: Failed to add permission for user '" << username << "'." << std::endl;
        return false;
    }

    if (!writeJsonToFile(DATABASE_PATH, jsonData)) {
        std::cerr << "Failed to update JSON file." << std::endl;
        return false;
    }

    return true;
}

bool DatabaseManager::addUserPermission(json& dbJson, const std::string& username, Permission permission) {
    if (!dbJson.contains("users")) {
        dbJson["users"] = json::array();
    }

    auto userIt = findUser(dbJson["users"], username);
    if (userIt == dbJson["users"].end()) {
        dbJson["users"].push_back({
            {"username", username},
            {"permissions", {permissionToString(permission)}}
        });
        return true;
    }

    if (!userIt->contains("permissions")) {
        (*userIt)["permissions"] = json::array();
    }

    
    std::string permissionStr = permissionToString(permission);
    auto& permissions = (*userIt)["permissions"];
    if (std::find(permissions.begin(), permissions.end(), permissionStr) == permissions.end()) {
        permissions.push_back(permissionStr);
    }

    return true;
}



void DatabaseManager::databaseNotFound(const std::string& dbName) const {
    std::cerr << "Error: Database '" << dbName << "' not found.\n";
}
