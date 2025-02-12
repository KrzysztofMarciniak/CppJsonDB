#include "logManager/logManager.h"

LogManager::LogManager() {
    logFile.open("data/logs.txt", std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Unable to open log file!" << std::endl;
    }
}

LogManager::~LogManager() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void LogManager::log(LogLevel level, const std::string& user, const std::string& database, const std::string& message) {
    if (!logFile.is_open()) {
        std::cerr << "Log file is not open!" << std::endl;
        return;
    }

    time_t now = std::time(nullptr);
    tm* timeinfo = std::localtime(&now);

    logFile << "[" << std::put_time(timeinfo, "%Y-%m-%d %H:%M:%S") << "] ";
    if (!user.empty()) logFile << "[" << user << "] ";
    if (!database.empty()) logFile << "[" << database << "] ";

    if (level == LogLevel::INFO) logFile << "[INFO] ";
    else if (level == LogLevel::WARNING) logFile << "[WARNING] ";
    else if (level == LogLevel::ERROR) logFile << "[ERROR] ";

    logFile << message << std::endl;
}
