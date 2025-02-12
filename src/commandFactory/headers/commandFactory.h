#ifndef COMMAND_FACTORY_H
#define COMMAND_FACTORY_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include "commands/command.h"
#include "userManager/userManager.h"
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>
#include <future>
    struct CommandInfo {
        std::string name;
        std::shared_ptr<Command> command;
        std::string description;
        std::vector<std::string> aliases;
    };
namespace fs = std::filesystem;
class CommandFactory {
public:
    CommandFactory(std::string& currentUser, std::string& currentDatabase);
    void registerCommand(const std::string& commandName, std::shared_ptr<Command> command, const std::string& description, const std::vector<std::string>& aliases = {});
    void registerMetaCommands(std::string& currentUser,std::string& currentDatabase);
    void registerSQLCommands(std::string& currentUser,std::string& currentDatabase);
    std::shared_ptr<Command> getCommand(const std::string& commandName) const;
    std::vector<std::string> getAllMetaCommands() const;
    std::vector<std::string> getMetaCommands(const std::string& searchPattern) const;
    std::string getCommandDescription(const std::string& commandName) const;
    std::vector<std::string> getCommandAliases(const std::string& commandName) const;
    std::vector<std::string> getAllCommands() const;
    void checkAndCreateFileSystem() const;
    void registerCommandAsync(const std::string& name, std::shared_ptr<Command> command, const std::string& description, const std::vector<std::string>& aliases);
private:
    std::vector<std::future<void>> futures;
    std::unordered_map<std::string, std::shared_ptr<Command>> commands;
    std::unordered_map<std::string, std::string> users;
    std::unordered_map<std::string, std::string> commandDescriptions;
    std::unordered_map<std::string, std::vector<std::string>> commandAliasMap;
    std::unordered_set<std::string> primaryCommands;
    std::string& currentUser;
    std::string& currentDatabase;

};
#define REGISTER_COMMANDS_ASYNC(commandsVector, factoryInstance) \
    std::vector<std::future<void>> futures; \
    for (const auto& cmd : commandsVector) { \
        futures.push_back(std::async(std::launch::async, \
            &CommandFactory::registerCommandAsync, factoryInstance, \
            cmd.name, cmd.command, cmd.description, cmd.aliases)); \
    } \
    for (auto& future : futures) { \
        future.get(); \
    }

#endif
