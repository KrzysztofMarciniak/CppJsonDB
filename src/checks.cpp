#include "checks.h"
#include <stdexcept>

void checkUserLoggedIn(const std::string& currentUser) {
    if (currentUser.empty()) {
        DISPLAY_MESSAGE("Error: You must be logged in to perform this action.");
        return;
    }
}

void checkDatabaseSelected(const std::string& currentUser, const std::string& currentDatabase) {
    if (currentDatabase.empty()) {
        DISPLAY_MESSAGE("Error: You must select a database to perform this action.");
        LOG(LogLevel::ERROR, currentUser, currentDatabase, "No database is currently selected.");
        return;
    }
}
