#include "lockManager/lockManager.h"

LockManager* LockManager::instance = nullptr;
std::mutex LockManager::singletonMutex;

LockManager& LockManager::getInstance() {
    if (instance == nullptr) {
        std::lock_guard<std::mutex> guard(singletonMutex);
        if (instance == nullptr) {
            instance = new LockManager();
        }
    }
    return *instance;
}

std::string LockManager::getLockFilePath(const std::string& resource, LockFileType type) {
    switch (type) {
        case LockFileType::DATABASE:
            return "data/databases/" + resource + ".lock";
        case LockFileType::USER_INFO:
            return "data/users.lock";
        case LockFileType::DATABASE_MANAGER:
            return "data/databases.lock";
        default:
            throw std::invalid_argument("Unknown lock file type");
    }
}


void LockManager::writeLockFile(const std::string& filePath, const std::string& user, const std::string& operation) {
    std::ofstream lockFile(filePath);
    if (!lockFile) {
        throw std::system_error(errno, std::generic_category(), "Failed to create lock file");
    }
    lockFile << user << "\n" << operation;
    lockFile.close();
}

bool LockManager::readLockFile(const std::string& filePath) {
    std::ifstream lockFile(filePath);
    std::string user, operation;
    if (!lockFile) {
        return false;
    }
    std::getline(lockFile, user);
    std::getline(lockFile, operation);
    std::cout << "user: " + user + " has locked file for operation: " + operation + "\n";
    return true;
}

void LockManager::removeLockFile(const std::string& filePath) {
    if (!std::filesystem::remove(filePath)) {
        throw std::system_error(errno, std::generic_category(), "Failed to remove lock file");
    }
}

void LockManager::lock(const std::string& resource, LockFileType type, const std::string& currentuser, const std::string& operation) {
    std::lock_guard<std::mutex> guard(singletonMutex);

    std::string filePath = getLockFilePath(resource, type);
    std::ofstream lockFile(filePath);
    if (lockFile.is_open()) {
        lockFile.close();
    } else {
        throw std::system_error(errno, std::generic_category(), "Failed to create lock file");
    }

    writeLockFile(filePath, currentuser, operation);
}

void LockManager::unlock(const std::string& resource, LockFileType type) {
    std::lock_guard<std::mutex> guard(singletonMutex);

    std::string filePath = getLockFilePath(resource, type);
    removeLockFile(filePath);
}

bool LockManager::isLocked(const std::string& resource, LockFileType type) {
    std::lock_guard<std::mutex> guard(singletonMutex);
    std::string user, operation;
    std::string filePath = getLockFilePath(resource, type);
    if (readLockFile(filePath)) {
        return true;
    } else {
        user.clear();
        operation.clear();
    }
    return false;
}