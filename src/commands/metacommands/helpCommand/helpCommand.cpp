#include "headers/helpCommand.h"

HelpCommand::HelpCommand(const CommandFactory& commandFactory) : commandFactory(commandFactory) {}

void HelpCommand::execute(const std::vector<std::string>& args) {
    std::vector<std::string> metaCommands = getMetaCommands(args);

    for (const auto& command : metaCommands) {
        std::string description = commandFactory.getCommandDescription(command);
        std::vector<std::string> aliases = commandFactory.getCommandAliases(command);

        printCommandDetails(command, aliases, description);
    }
}

std::vector<std::string> HelpCommand::getMetaCommands(const std::vector<std::string>& args) const {
    if (args.empty()) {
        return commandFactory.getAllMetaCommands();
    } else {
        return commandFactory.getMetaCommands(args[0]);
    }
}

void HelpCommand::printCommandDetails(const std::string& command, const std::vector<std::string>& aliases, const std::string& description) const {
    std::cout << command;

    if (!aliases.empty()) {
        printAliases(aliases);
    }

    std::cout << " - " << description << std::endl;
}

void HelpCommand::printAliases(const std::vector<std::string>& aliases) const {
    std::cout << " (aliases: ";
    for (size_t i = 0; i < aliases.size(); ++i) {
        std::cout << aliases[i];
        if (i < aliases.size() - 1) {
            std::cout << ", ";
        }
    }
    std::cout << ")";
}
