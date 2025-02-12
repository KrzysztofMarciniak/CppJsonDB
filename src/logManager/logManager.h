#ifndef LOG_MANAGER_H
#define LOG_MANAGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <iomanip>

enum class LogLevel {
    INFO,
    WARNING,
    ERROR
};

class LogManager {
public:
    static LogManager& getInstance() {
        static LogManager instance; 
        return instance;
    }

    void log(LogLevel level, const std::string& user, const std::string& database, const std::string& message);

private:
    LogManager(); 
    ~LogManager();

    LogManager(const LogManager&) = delete; 
    LogManager& operator=(const LogManager&) = delete;

    std::ofstream logFile;
};

#define LOG(level, user, database, message) \
    do { \
        LogManager::getInstance().log(level, user, database, message); \
    } while (false)

#define DISPLAY_MESSAGE(message) \
    do { \
        std::cout << message << std::endl; \
    } while (false)

#endif
