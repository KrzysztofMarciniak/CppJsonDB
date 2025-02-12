#include "headers/describeCommand.h"

DescribeCommand::DescribeCommand(std::string &currentUser, std::string &currentDatabase)
: currentUser(currentUser), currentDatabase(currentDatabase) {}

void DescribeCommand::execute(const std::vector<std::string>& args) {
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

    if (args.size() != 1) {
        std::cerr << "Error: Invalid arguments for DESCRIBE command." << std::endl;
        return;
    }

    if (IS_LOCKED(currentDatabase, LockFileTypeToUse)){
        DISPLAY_MESSAGE(currentDatabase + " is currently locked... try again later");
        return;
    }
    LOCK(currentDatabase, LockFileTypeToUse, currentUser, "DESCRIBE");

    std::string tableName = args[0];
    std::cout << "Describing table '" << tableName << "' in database '" << currentDatabase << "'." << std::endl;

    json dbContent;
    if (!sqlManager.readDatabaseFile(currentDatabase, dbContent)) {
        std::cerr << "Error: Database '" << currentDatabase << "' does not exist." << std::endl;
        UNLOCK(currentDatabase, LockFileTypeToUse);
        return;
    }

    json table = sqlManager.findTable(dbContent, tableName);
    if (table.is_null()) {
        std::cerr << "Error: Table '" << tableName << "' does not exist in database '" << currentDatabase << "'." << std::endl;
        UNLOCK(currentDatabase, LockFileTypeToUse);
        return;
    }

    std::cout << "Table: " << tableName << std::endl;
    std::cout << "Columns:" << std::endl;

    for (const auto& column : table["columns"]) {
        std::string columnName = column["name"];
        std::string columnType = column["type"];
        std::cout << "  " << columnName << " " << columnType << std::endl;
    }
    UNLOCK(currentDatabase, LockFileTypeToUse);

}
