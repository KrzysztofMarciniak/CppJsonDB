#include "headers/useDatabaseCommand.h"
#include "logManager/logManager.h"
#include "databaseManager/databaseManager.h"
#include <iostream>
#include "lockManager/lockManager.h"

UseDatabaseCommand::UseDatabaseCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase) {}

void UseDatabaseCommand::execute(const std::vector<std::string>& args) {
    checkUserLoggedIn(currentUser);
    checkDatabaseSelected(currentUser,currentDatabase);
    if (args.size() != 1) {
        DISPLAY_MESSAGE("Usage: .use <database name>");
        return;
    }

    INIT_DB_MANAGER;

    std::string toUseDatabase = args[0];
    LockFileType LockFileTypeToUse = LockFileType::DATABASE_MANAGER;
    LOCK("",LockFileTypeToUse,currentUser,"use command");
    if (!dbManager.databaseExists(toUseDatabase)) {
        UNLOCK("",LockFileTypeToUse);
        LOG(LogLevel::ERROR, toUseDatabase, currentUser, "Database does not exist.");
        DISPLAY_MESSAGE("Error: Database '" + toUseDatabase + "' does not exist.");
        return;
    }

    if (!dbManager.canUseDatabase(toUseDatabase, currentUser)) {
        UNLOCK("",LockFileTypeToUse);
        LOG(LogLevel::ERROR, toUseDatabase, currentUser, "User '" + currentUser + "' does not have permission to use database '" + toUseDatabase + "'.");
        DISPLAY_MESSAGE("Error: You do not have permission to use database '" + toUseDatabase + "'.");
        return;
    }
    UNLOCK("",LockFileTypeToUse);
    currentDatabase = toUseDatabase;
    LOG(LogLevel::INFO, currentDatabase, currentUser, "Switched to database: " + currentDatabase);

    DISPLAY_MESSAGE("Switched to database: " + currentDatabase);
}
