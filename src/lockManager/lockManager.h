#ifndef LOCK_MANAGER_H
#define LOCK_MANAGER_H

#include <string>
#include <mutex>
#include <unordered_map>
#include <memory>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <map>
#include <mutex>
#include <string>
#include <system_error>
#include <filesystem>

enum class LockFileType {
    DATABASE,           // data/databases/(databasename).lock - sqlManager
    USER_INFO,          // data/users.lock - userManager
    DATABASE_MANAGER    // data/databases.lock - dbManager
};


class LockManager {
public:
    static LockManager& getInstance();

    void lock(const std::string& resource, LockFileType type, const std::string& currentuser, const std::string& operation);
    void unlock(const std::string& resource, LockFileType type);
    bool isLocked(const std::string& resource, LockFileType type);

private:
    LockManager() = default;
    LockManager(const LockManager&) = delete;
    LockManager& operator=(const LockManager&) = delete;

    std::shared_ptr<std::mutex> getMutexForResource(const std::string& resource);

    static LockManager* instance;
    static std::mutex singletonMutex;

    std::unordered_map<std::string, std::shared_ptr<std::mutex>> mutexes;

    std::string getLockFilePath(const std::string& resource, LockFileType type);
    void writeLockFile(const std::string& filePath, const std::string& user, const std::string& operation);
    bool readLockFile(const std::string& filePath);
    void removeLockFile(const std::string& filePath);

};


#define IS_LOCKED(resource, type) (LockManager::getInstance().isLocked(resource, type))
#define LOCK(resource, type, currentuser, operation)      (LockManager::getInstance().lock(resource, type, currentuser, operation))
#define UNLOCK(resource, type)    (LockManager::getInstance().unlock(resource, type))

#endif

