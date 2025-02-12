#ifndef USE_DATABASE_COMMAND_H
#define USE_DATABASE_COMMAND_H

#include "checks.h"
#include "commands/command.h"
#include <string>
class UseDatabaseCommand : public Command {
public:
    UseDatabaseCommand(std::string& currentUser, std::string& currentDatabase);

    void execute(const std::vector<std::string>& args) override;

private:
    std::string& currentUser;
    std::string& currentDatabase;
};

#endif
