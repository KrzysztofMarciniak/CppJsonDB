#include "headers/addPermissionCommand.h"

AddPermissionCommand::AddPermissionCommand(std::string& currentUser)
    : currentUser(currentUser) {}

void AddPermissionCommand::execute(const std::vector<std::string>& args) {
    checkUserLoggedIn(currentUser);
    LockFileType LockFileTypeToUse = LockFileType::DATABASE_MANAGER;
    if (args.size() < 3) {
        DISPLAY_MESSAGE("Usage: .add permission <databaseName> <username> <permission1> [<permission2> ...], READ, WRITE, ADD_TO_TABLE, MANAGE_PERMISSIONS");
        return;
    }

    std::string databaseName = args[0];
    std::string username = args[1];

    INIT_DB_MANAGER;

    if (!dbManager.databaseExists(databaseName)) {
        LOG(LogLevel::ERROR, databaseName, currentUser, "Database '" + databaseName + "' not found.");
        DISPLAY_MESSAGE("Error: Database '" + databaseName + "' not found.");
        return;
    }

    if (!dbManager.canManagePermissions(databaseName, currentUser)) {
        LOG(LogLevel::ERROR, databaseName, currentUser, "User '" + currentUser + "' does not have permission to manage permissions for database '" + databaseName + "'.");
        DISPLAY_MESSAGE("Error: You do not have permission to manage permissions for this database.");
        return;
    }

    bool allPermissionsAdded = true;


    if (IS_LOCKED("", LockFileTypeToUse)){
        DISPLAY_MESSAGE("databases config file is currently locked... try again later");
        return;
    }
    LOCK("", LockFileTypeToUse, currentUser, "ADD PERMISSION");
    for (size_t i = 2; i < args.size(); ++i) {
        const std::string& permissionStr = args[i];
        Permission permission;

        try {
            permission = dbManager.stringToPermission(permissionStr);
        } catch (const std::invalid_argument& e) {
            LOG(LogLevel::ERROR, databaseName, currentUser, e.what());
            DISPLAY_MESSAGE(e.what());
            allPermissionsAdded = false;
            continue;
        }

        if (!dbManager.addPermission(databaseName, username, permission)) {
            LOG(LogLevel::ERROR, databaseName, currentUser, "Failed to add permission '" + permissionStr + "' for user '" + username + "'.");
            DISPLAY_MESSAGE("Failed to add permission '" + permissionStr + "'.");
            allPermissionsAdded = false;
        }
    }
    UNLOCK("", LockFileTypeToUse);
    if (allPermissionsAdded) {
        LOG(LogLevel::INFO, databaseName, currentUser, "Permissions added successfully for user '" + username + "'.");
        DISPLAY_MESSAGE("Permissions added successfully.");
    } else {
        LOG(LogLevel::ERROR, databaseName, currentUser, "Some permissions could not be added for user '" + username + "'.");
        DISPLAY_MESSAGE("Some permissions could not be added.");
    }
}
