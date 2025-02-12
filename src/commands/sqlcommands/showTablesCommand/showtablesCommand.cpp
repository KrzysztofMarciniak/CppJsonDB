#include "headers/showTablesCommand.h"

ShowTablesCommand::ShowTablesCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase){}

void ShowTablesCommand::execute(const std::vector<std::string>& args) {
    INIT_DB_MANAGER;
    INIT_SQL_MANAGER;
    LockFileType LockFileTypeToUse = LockFileType::DATABASE;
    checkUserLoggedIn(currentUser);
    checkDatabaseSelected(currentUser, currentDatabase);

    bool isOwner = IS_OWNER(currentDatabase, currentUser);
    bool hasReadPermissions = CHECK_USER_PERMISSION(currentDatabase, currentUser, Permission::READ);

    if (!isOwner && !hasReadPermissions) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has read permission for database '" + currentDatabase + "'.");
        return;
    }
    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }
    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "SHOW TABLES");

    json dbContent;
    if (!sqlManager.readDatabaseFile(currentDatabase, dbContent)) {
        std::cerr << "Error: Database '" << currentDatabase << "' does not exist." << std::endl;
        UNLOCK(currentDatabase, LockFileTypeToUse);
        return;
    }

    auto& tables = dbContent["tables"];
    if (tables.empty()) {
        std::cout << "No tables found in database '" << currentDatabase << "'." << std::endl;
    } else {
        std::cout << "Tables in database '" << currentDatabase << "':" << std::endl;
        for (const auto& table : tables) {
            std::string tableName = table["table_name"];
            std::cout << "  " << tableName << std::endl;
        }
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);
}
