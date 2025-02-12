#ifndef HELP_COMMAND_H
#define HELP_COMMAND_H

#include "commands/command.h"
#include "commandFactory/headers/commandFactory.h"
#include <memory>
#include <iostream>
#include <ostream>
#include <vector>
#include <string>
class HelpCommand : public Command {
public:
    HelpCommand(const CommandFactory& commandFactory);
    void execute(const std::vector<std::string>& args) override;

private:
    const CommandFactory& commandFactory;

    std::vector<std::string> getMetaCommands(const std::vector<std::string>& args) const;
    void printCommandDetails(const std::string& command, const std::vector<std::string>& aliases, const std::string& description) const;
    void printAliases(const std::vector<std::string>& aliases) const;
};

#endif
