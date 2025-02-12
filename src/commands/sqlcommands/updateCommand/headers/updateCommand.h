#ifndef UPDATE_COMMAND_H
#define UPDATE_COMMAND_H

#include "commands/command.h"
#include <string>
#include <vector>
#include <map>

class UpdateCommand : public Command {
public:
    UpdateCommand(std::string& currentUser, std::string& currentDatabase);
    virtual void execute(const std::vector<std::string>& args) override;

private:
    std::string& currentUser;
    std::string& currentDatabase;

    std::map<std::string, std::map<std::string, std::vector<std::string>>> parseArgsForUpdate(std::vector<std::string>& args);
    bool checkPermissions();
};

#endif
