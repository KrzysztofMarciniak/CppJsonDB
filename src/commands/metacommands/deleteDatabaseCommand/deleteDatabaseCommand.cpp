#include "headers/deleteDatabaseCommand.h"
#include "sqlManager/sqlManager.h"
#include "lockManager/lockManager.h"
DeleteDatabaseCommand::DeleteDatabaseCommand(std::string& currentUser)
    : currentUser(currentUser) {}

void DeleteDatabaseCommand::execute(const std::vector<std::string>& args) {
    checkUserLoggedIn(currentUser);
    if (args.empty()) {
        DISPLAY_MESSAGE("Error: No database names provided.");
        return;
    }

    INIT_DB_MANAGER;
    INIT_SQL_MANAGER;

    for (const std::string& dbName : args) {
        if (!dbManager.databaseExists(dbName)) {
            DISPLAY_MESSAGE("Error: Database '" + dbName + "' does not exist.");
            LOG(LogLevel::ERROR, dbName, currentUser, "Attempted to delete non-existent database '" + dbName + "'.");
            continue;
        }

        if (!dbManager.isOwner(dbName, currentUser)) {
            DISPLAY_MESSAGE("Error: User '" + currentUser + "' is not the owner of '" + dbName + "'.");
            LOG(LogLevel::ERROR, dbName, currentUser, "User '" + currentUser + "' attempted to delete database '" + dbName + "' without ownership.");
            continue;
        }

        if (IS_LOCKED(dbName, LockFileType::DATABASE_MANAGER)) {
            DISPLAY_MESSAGE(dbName + " config file is currently locked... try again later");
            continue;
        }
        if (IS_LOCKED(dbName, LockFileType::DATABASE)) {
            DISPLAY_MESSAGE(dbName + " file is currently locked... try again later");
            continue;
        }

        LOCK(dbName, LockFileType::DATABASE, currentUser, "Delete DATABASE - database file");
        LOCK(dbName, LockFileType::DATABASE_MANAGER, currentUser, "Delete DATABASE - database config file");

        bool lockReleased = false;
        try {
            if (dbManager.deleteDatabase(dbName)) {
                sqlManager.deleteDatabase(dbName);
                DISPLAY_MESSAGE("Database '" + dbName + "' deleted successfully.");
                LOG(LogLevel::INFO, dbName, currentUser, "Database '" + dbName + "' deleted successfully by user '" + currentUser + "'.");
            } else {
                DISPLAY_MESSAGE("Error: Failed to delete database '" + dbName + "'.");
                LOG(LogLevel::ERROR, dbName, currentUser, "Failed to delete database '" + dbName + "' by user '" + currentUser + "'.");
            }
        } catch (...) {
            DISPLAY_MESSAGE("An unexpected error occurred while deleting database '" + dbName + "'.");
            LOG(LogLevel::ERROR, dbName, currentUser, "Unexpected error while deleting database '" + dbName + "'.");
        }

        UNLOCK(dbName, LockFileType::DATABASE_MANAGER);
        UNLOCK(dbName, LockFileType::DATABASE);
        lockReleased = true;

    }
}
