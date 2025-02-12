#include "headers/removePermissionCommand.h"
#include "lockManager/lockManager.h"
RemovePermissionCommand::RemovePermissionCommand(std::string& currentUser)
    : currentUser(currentUser) {}

void RemovePermissionCommand::execute(const std::vector<std::string>& args) {
    checkUserLoggedIn(currentUser);

    if (args.size() < 3) {
        DISPLAY_MESSAGE("Usage: .remove permission <databaseName> <username> <permission1> [<permission2> ...], READ WRITE ADD_TO_TABLE MANAGE_PERMISSIONS");
        return;
    }

    std::string databaseName = args[0];
    std::string username = args[1];

    INIT_DB_MANAGER;
    LockFileType LockFileTypeToUse = LockFileType::DATABASE_MANAGER;
    if (IS_LOCKED("",LockFileTypeToUse)){
        DISPLAY_MESSAGE("databases config file is currently locked... try again later");
        return;
    }
    LOCK("",LockFileTypeToUse,currentUser,"Remove Permission");
    if (!dbManager.databaseExists(databaseName)) {
        UNLOCK("",LockFileTypeToUse);
        DISPLAY_MESSAGE("Error: Database '" + databaseName + "' not found.");
        LOG(LogLevel::ERROR, databaseName, currentUser, "Database '" + databaseName + "' not found.");
        return;
    }

    if (!dbManager.canManagePermissions(databaseName, currentUser)) {
        UNLOCK("",LockFileTypeToUse);
        DISPLAY_MESSAGE("Error: You do not have permission to manage permissions for this database.");
        LOG(LogLevel::ERROR, databaseName, currentUser, "User '" + currentUser + "' does not have permission to manage permissions for database '" + databaseName + "'.");
        return;
    }

    bool allPermissionsRemoved = true;

    for (size_t i = 2; i < args.size(); ++i) {
        const std::string& permissionStr = args[i];
        Permission permission;

        try {
            permission = dbManager.stringToPermission(permissionStr);
        } catch (const std::invalid_argument& e) {
            DISPLAY_MESSAGE(e.what());
            LOG(LogLevel::ERROR, databaseName, currentUser, e.what());
            allPermissionsRemoved = false;
            continue;
        }

        if (!dbManager.removeUserPermission(databaseName, username, permissionStr)) {
            DISPLAY_MESSAGE("Failed to remove permission '" + permissionStr + "'.");
            LOG(LogLevel::ERROR, databaseName, currentUser, "Failed to remove permission '" + permissionStr + "' for user '" + username + "'.");
            allPermissionsRemoved = false;
        }
    }

    if (allPermissionsRemoved) {
        DISPLAY_MESSAGE("Permissions removed successfully.");
        LOG(LogLevel::INFO, databaseName, currentUser, "Permissions removed successfully for user '" + username + "'.");
    } else {
        DISPLAY_MESSAGE("Some permissions could not be removed.");
        LOG(LogLevel::ERROR, databaseName, currentUser, "Some permissions could not be removed for user '" + username + "'.");
    }
    UNLOCK("",LockFileTypeToUse);
}
