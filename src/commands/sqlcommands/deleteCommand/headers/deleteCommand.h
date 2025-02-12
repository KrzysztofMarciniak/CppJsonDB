#ifndef DELETE_COMMAND_H
#define DELETE_COMMAND_H

#include "commands/command.h"
#include <iostream>
#include <map>
class DeleteCommand : public Command {
public:
    DeleteCommand(std::string& currentUser, std::string& currentDatabase);
    virtual void execute(const std::vector<std::string>& args) override;
    std::map<std::string, std::map<std::string, std::vector<std::string>>> parseArgsForDeletion(std::vector<std::string>& args);

private:
    bool checkPermissions();
    std::string& currentUser;
    std::string& currentDatabase;
};

#endif
