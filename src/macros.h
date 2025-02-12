#ifndef MACROS_H
#define MACROS_H
#include "logManager/logManager.h"
#define CHECK_USER_LOGGED_IN(currentUser) \
    if (currentUser.empty()) { \
        DISPLAY_MESSAGE("Error: You must be logged in to perform this action."); \
        return; \
    }
#define CHECK_DATABASE(currentDatabase, currentUser) \
    if (currentDatabase.empty()) { \
        DISPLAY_MESSAGE("Error: You must select a database to perform this action."); \
        LOG(LogLevel::ERROR, currentUser, currentDatabase, "No database is currently selected."); \
        return; \
    }
#endif
