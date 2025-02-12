#include "headers/viewAllDatabasePermissionsCommand.h"

ViewAllDatabasePermissionsCommand::ViewAllDatabasePermissionsCommand(std::string& currentUser)
    : currentUser(currentUser) {
}

void ViewAllDatabasePermissionsCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 1 || args.size() > 1) {
        DISPLAY_MESSAGE("Usage: .view database permissions <databaseName>");
        return;
    }

    std::string databaseName = args[0];

    INIT_DB_MANAGER;
    auto databasePermissionsMap = dbManager.getDatabasePermissions(databaseName);

    if (databasePermissionsMap.find(databaseName) == databasePermissionsMap.end()) {
        DISPLAY_MESSAGE("Database '" + databaseName + "' does not exist, is private or you don't have permission to view it.");
        LOG(LogLevel::ERROR, databaseName, currentUser, "Database '" + databaseName + "' does not exist, is private or permission denied.");
        return;
    }

    const auto& databasePermissions = databasePermissionsMap.at(databaseName);

    for (const auto& userPermissions : databasePermissions) {
        const std::string& username = userPermissions.first;
        const std::unordered_set<Permission>& permissions = userPermissions.second;

        LOG(LogLevel::INFO, databaseName, currentUser, "Viewing permissions for user '" + username + "' in database '" + databaseName + "'");
        DISPLAY_MESSAGE(username);

        for (const auto& permission : permissions) {
            LOG(LogLevel::INFO, databaseName, currentUser, "  - Permission: " + dbManager.permissionToString(permission));
            DISPLAY_MESSAGE("  - Permission: " + dbManager.permissionToString(permission));
        }
    }
}
