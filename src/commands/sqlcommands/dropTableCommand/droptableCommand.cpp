#include "headers/dropTableCommand.h"

DropTableCommand::DropTableCommand(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase){}

void DropTableCommand::execute(const std::vector<std::string>& args) {
    INIT_DB_MANAGER;
    INIT_SQL_MANAGER;
    LockFileType LockFileTypeToUse = LockFileType::DATABASE;
    checkUserLoggedIn(currentUser);
    checkDatabaseSelected(currentUser, currentDatabase);

    bool isOwner = IS_OWNER(currentDatabase, currentUser);
    bool hasWritePermissions = CHECK_USER_PERMISSION(currentDatabase, currentUser, Permission::WRITE);

    if (!isOwner && !hasWritePermissions) {
        DISPLAY_MESSAGE("Current user is neither the owner nor has read permission for database '" + currentDatabase + "'.");
        return;
    }
        if (args.size() != 1) {
        std::cerr << "Error: Invalid arguments for DROP TABLE command." << std::endl;
        return;
    }

    std::string tableName = args[0];

    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }
    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "DROP TABLE");
    json dbContent;
    if (!sqlManager.readDatabaseFile(currentDatabase, dbContent)) {
        std::cerr << "Error: Failed to read database file for database '" << currentDatabase << "'." << std::endl;
        UNLOCK(currentDatabase, LockFileTypeToUse);
        return;
    }

    auto tableIt = std::find_if(dbContent["tables"].begin(), dbContent["tables"].end(),
                                [&tableName](const json& table) { return table["table_name"] == tableName; });

    if (tableIt == dbContent["tables"].end()) {
        std::cerr << "Error: Table '" << tableName << "' does not exist in database '" << currentDatabase << "'." << std::endl;
        UNLOCK(currentDatabase, LockFileTypeToUse);
        return;
    }

    dbContent["tables"].erase(tableIt);

    if (!sqlManager.writeDatabaseFile(currentDatabase, dbContent)) {
        std::cerr << "Error: Failed to write database file for database '" << currentDatabase << "'." << std::endl;
        UNLOCK(currentDatabase, LockFileTypeToUse);
        return;
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);
}
