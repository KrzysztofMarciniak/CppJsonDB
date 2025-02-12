#include "headers/createDatabaseCommand.h"
#include "sqlManager/sqlManager.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include "lockManager/lockManager.h"

CreateDatabaseCommand::CreateDatabaseCommand(std::string& currentUser)
    : currentUser(currentUser) {}

void CreateDatabaseCommand::execute(const std::vector<std::string>& args) {
    checkUserLoggedIn(currentUser);
    LockFileType LockFileTypeToUse = LockFileType::DATABASE_MANAGER;
    if (args.size() < 1) {
        DISPLAY_MESSAGE("Usage: .create database <databaseName1> [<databaseName2> ...]");
        return;
    }

    INIT_DB_MANAGER;
    INIT_SQL_MANAGER;

    bool allCreated = true;
    if (IS_LOCKED("", LockFileTypeToUse)){
        DISPLAY_MESSAGE("databases config file is currently locked... try again later");
        return;
    }
    LOCK("", LockFileTypeToUse, currentUser, "CREATE DATABASE");
    for (const std::string& databaseName : args) {
        if (dbManager.createDatabase(databaseName, currentUser)) {
            if (sqlManager.createDatabase(databaseName)) {
                LOG(LogLevel::INFO, "", currentUser, "Database created: " + databaseName);
                DISPLAY_MESSAGE("Database created: " + databaseName);
            } else {
                LOG(LogLevel::ERROR, "", currentUser, "Error: Could not create database file for '" + databaseName + "'");
                DISPLAY_MESSAGE("Error: Could not create database file for '" + databaseName + "'");
                allCreated = false;
            }
        } else {
            LOG(LogLevel::ERROR, "", currentUser, "Error: Could not create database '" + databaseName + "'");
            DISPLAY_MESSAGE("Error: Could not create database '" + databaseName + "'");
            allCreated = false;
        }
    }
    if (!allCreated) {
        DISPLAY_MESSAGE("Some databases could not be created.");
    }
    UNLOCK("", LockFileTypeToUse);
}
