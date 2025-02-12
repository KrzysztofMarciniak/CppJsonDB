#include "headers/commandFactory.h"

#include "commands/metacommands/exitCommand/headers/exitCommand.h"
#include "commands/metacommands/setDummyUserCommand/headers/setDummyUserCommand.h"
#include "commands/metacommands/setDummyDatabaseCommand/headers/setDummyDatabaseCommand.h"
#include "commands/metacommands/helpCommand/headers/helpCommand.h"
#include "commands/metacommands/createUserCommand/headers/createUserCommand.h"
#include "commands/metacommands/loginCommand/headers/loginCommand.h"
#include "commands/metacommands/logoutCommand/headers/logoutCommand.h"
#include "commands/metacommands/createDatabaseCommand/headers/createDatabaseCommand.h"
#include "commands/metacommands/listDatabasesCommand/headers/listDatabasesCommand.h"
#include "commands/metacommands/addPermissionCommand/headers/addPermissionCommand.h"
#include "commands/metacommands/removePermissionCommand/headers/removePermissionCommand.h"
#include "commands/metacommands/viewAllDatabasePermissionsCommand/headers/viewAllDatabasePermissionsCommand.h"
#include "commands/metacommands/deleteDatabaseCommand/headers/deleteDatabaseCommand.h"
#include "commands/metacommands/useDatabaseCommand/headers/useDatabaseCommand.h"

#include "commands/sqlcommands/selectCommand/headers/selectCommand.h"
#include "commands/sqlcommands/insertCommand/headers/insertCommand.h"
#include "commands/sqlcommands/createTableCommand/headers/createTableCommand.h"
#include "commands/sqlcommands/deleteCommand/headers/deleteCommand.h"
#include "commands/sqlcommands/describeCommand/headers/describeCommand.h"
#include "commands/sqlcommands/dropTableCommand/headers/dropTableCommand.h"
#include "commands/sqlcommands/showTablesCommand/headers/showTablesCommand.h"
#include "commands/sqlcommands/updateCommand/headers/updateCommand.h"


CommandFactory::CommandFactory(std::string& currentUser, std::string& currentDatabase)
    : currentUser(currentUser), currentDatabase(currentDatabase) {
    checkAndCreateFileSystem();

    auto metaCommandFuture = std::async(std::launch::async, &CommandFactory::registerMetaCommands, this, std::ref(currentUser), std::ref(currentDatabase));
    auto sqlCommandFuture = std::async(std::launch::async, &CommandFactory::registerSQLCommands, this, std::ref(currentUser), std::ref(currentDatabase));

    metaCommandFuture.get();
    sqlCommandFuture.get();
}

void CommandFactory::checkAndCreateFileSystem() const {
    std::vector<std::string> requiredFiles = {
        "data/users.json",
        "data/logs.txt",
        "data/databases.json"
    };

    if (!fs::exists("data")) {
        if (!fs::create_directory("data")) {
            throw std::runtime_error("Failed to create directory: data");
        }
    }

    if (!fs::exists("data/databases")) {
        if (!fs::create_directory("data/databases")) {
            throw std::runtime_error("Failed to create directory: data/databases");
        }
    }

    for (const auto& file : requiredFiles) {
        if (!fs::exists(file)) {
            std::ofstream newFile(file);
            if (!newFile) {
                throw std::runtime_error("Failed to create file: " + file);
            }
            if (file.ends_with(".json")) {
                json data = json::object();
                newFile << data.dump();
            }
            newFile.close();
        }
    }
}
void CommandFactory::registerCommandAsync(const std::string& name, std::shared_ptr<Command> command, const std::string& description, const std::vector<std::string>& aliases) {
    std::async(std::launch::async, &CommandFactory::registerCommand, this, name, command, description, aliases);
}


void CommandFactory::registerMetaCommands(std::string& currentUser, std::string& currentDatabase) {

    std::vector<CommandInfo> commands = {
        {".exit", std::make_shared<ExitCommand>(), "Exit the application", {".quit", ".q"}},
        {".set dummy user", std::make_shared<SetDummyUserCommand>(currentUser), "Set a dummy user, .set dummy user <user>"},
        {".set dummy database", std::make_shared<SetDummyDatabaseCommand>(currentDatabase), "Set a dummy database, .set dummy database <database>"},
        {".help", std::make_shared<HelpCommand>(*this), "Display help information", {".h"}},
        {".login", std::make_shared<LoginCommand>(currentUser), "Login command, .login <user> <password>"},
        {".logout", std::make_shared<LogoutCommand>(currentUser, currentDatabase), "Logout command, .logout"},
        {".create user", std::make_shared<CreateUserCommand>(), "Create user command, .create user <username> <password>"},
        {".create database", std::make_shared<CreateDatabaseCommand>(currentUser), "Create database command, .create database <databaseName>", {".c db"}},
        {".add permission", std::make_shared<AddPermissionCommand>(currentUser), "Add permission to database, .add permission <databaseName> <username> <permission>", {".add p"}},
        {".list databases", std::make_shared<ListDatabasesCommand>(), "List all databases", {".ls dbs"}},
        {".remove permission", std::make_shared<RemovePermissionCommand>(currentUser), "Remove permission from database, .remove permission <databaseName> <username> <permission>", {".rm p"}},
        {".view database permissions", std::make_shared<ViewAllDatabasePermissionsCommand>(currentUser), "View permissions for database, .view database permissions <databaseName>", {".ls db p"}},
        {".delete database", std::make_shared<DeleteDatabaseCommand>(currentUser), "Delete a database, .delete database <databaseName>", {".rm db"}},
        {".use", std::make_shared<UseDatabaseCommand>(currentUser, currentDatabase), "use database command, .use <database name>"}
    };

    REGISTER_COMMANDS_ASYNC(commands, this);
}

void CommandFactory::registerSQLCommands(std::string& currentUser, std::string& currentDatabase) {
    std::vector<CommandInfo> commands = {
        {"SELECT", std::make_shared<SelectCommand>(currentUser, currentDatabase), "Select command", {"select"}},
        {"INSERT", std::make_shared<InsertCommand>(currentUser, currentDatabase), "Insert command", {"insert"}},
        {"UPDATE", std::make_shared<UpdateCommand>(currentUser, currentDatabase), "Update command", {"update"}},
        {"DELETE", std::make_shared<DeleteCommand>(currentUser, currentDatabase), "Delete command", {"delete"}},
        {"CREATE TABLE", std::make_shared<CreateTableCommand>(currentUser, currentDatabase), "Create table command", {"create tbl", "create table"}},
        {"DROP TABLE", std::make_shared<DropTableCommand>(currentUser, currentDatabase), "Drop table command", {"drop tbl", "drop table"}},
        {"SHOW TABLES", std::make_shared<ShowTablesCommand>(currentUser, currentDatabase), "Show tables command", {"show tables"}},
        {"DESCRIBE", std::make_shared<DescribeCommand>(currentUser, currentDatabase), "Describe command", {"desc", "describe"}}
    };

    REGISTER_COMMANDS_ASYNC(commands, this);
}


void CommandFactory::registerCommand(const std::string& commandName, std::shared_ptr<Command> command, const std::string& description, const std::vector<std::string>& aliases) {
    commands.emplace(commandName, command);
    primaryCommands.insert(commandName);

    for (const auto& alias : aliases) {
        commands.emplace(alias, command);
        commandAliasMap[commandName].push_back(alias);
    }
    commandDescriptions[commandName] = description;
}

std::shared_ptr<Command> CommandFactory::getCommand(const std::string& commandName) const {
    auto it = commands.find(commandName);
    return (it != commands.end()) ? it->second : nullptr;
}

std::vector<std::string> CommandFactory::getAllCommands() const {
    std::vector<std::string> allCommands;
    for (const auto& command : commands) {
        allCommands.push_back(command.first);
    }
    return allCommands;
}

std::vector<std::string> CommandFactory::getAllMetaCommands() const {
    std::vector<std::string> metaCommands;
    for (const auto& command : primaryCommands) {
        if (command.find(".") == 0) {
            metaCommands.push_back(command);
        }
    }
    return metaCommands;
}

std::vector<std::string> CommandFactory::getMetaCommands(const std::string& searchPattern) const {
    std::vector<std::string> metaCommands;

    for (const auto& command : primaryCommands) {
        if (command.find(searchPattern) == 0) {
            metaCommands.push_back(command);
        }
    }
    for (const auto& entry : commandAliasMap) {
        for (const auto& alias : entry.second) {
            if (alias.find(searchPattern) == 0) {
                metaCommands.push_back(entry.first);
                break;
            }
        }
    }
    return metaCommands;
}

std::string CommandFactory::getCommandDescription(const std::string& commandName) const {
    auto it = commandDescriptions.find(commandName);
    return (it != commandDescriptions.end()) ? it->second : "No description available.";
}

std::vector<std::string> CommandFactory::getCommandAliases(const std::string& commandName) const {
    auto it = commandAliasMap.find(commandName);

    if (it != commandAliasMap.end()) {
        return it->second;
    } else {
        return std::vector<std::string>();
    }
}
