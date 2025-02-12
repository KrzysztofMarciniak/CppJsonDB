#include "userManager/userManager.h"
UserManager& UserManager::getInstance() {
    static UserManager instance;
    return instance;
}

UserManager::UserManager() {
    loadFromFile();
}

bool UserManager::createUser(const std::string& username, const std::string& password) {
    if (users.find(username) != users.end()) {
        std::cout << "Error: Username '" << username << "' already exists.\n";
        return false;
    }

    std::string encryptedPassword = hashPassword(password);
    if (encryptedPassword.empty()) {
        std::cout << "Error: Failed to encrypt password.\n";
        return false;
    }

    users[username] = { username, encryptedPassword };
    saveToFile();
    return true;
}

bool UserManager::login(const std::string& username, const std::string& password) {
    auto it = users.find(username);
    if (it == users.end()) {
        return false;
    }

    if (checkPassword(it->second.encryptedPassword, password)) {
        return true;
    } else {
        return false;
    }
    
}

void UserManager::saveToFile() {
    json j;
    for (const auto& pair : users) {
        json userJson;
        userJson["username"] = pair.second.username;
        userJson["encryptedPassword"] = pair.second.encryptedPassword;
        j[pair.first] = userJson;
    }

    std::ofstream file(usersFile);
    if (file.is_open()) {
        file << j.dump(3);
        file.close();
        std::cout << "Users saved to file: " << usersFile << "\n";
    } else {
        std::cerr << "Error: Unable to open file for writing: " << usersFile << "\n";
        std::cerr << "Error code: " << errno << " (" << strerror(errno) << ")\n";
    }
}


bool UserManager::loadFromFile() {
    std::ifstream file(usersFile);
    if (!file.is_open()) {
        return false;
    }

    json j;
    file >> j;
    file.close();

    for (json::iterator it = j.begin(); it != j.end(); ++it) {
        std::string username = it.key();
        std::string encryptedPassword = it.value()["encryptedPassword"];
        users[username] = { username, encryptedPassword };
    }

    return true;
}

std::string UserManager::hashPassword(const std::string& password) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, password.c_str(), password.size());
    SHA256_Final(hash, &sha256);

    std::stringstream ss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }

    return ss.str();
}
bool UserManager::checkPassword(const std::string& storedHashedPassword, const std::string& password) {
    std::string hashedPassword = hashPassword(password);
    return (storedHashedPassword == hashedPassword);
}
