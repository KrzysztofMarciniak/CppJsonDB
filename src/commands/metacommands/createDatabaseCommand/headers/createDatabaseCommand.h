#ifndef CREATE_DATABASE_COMMAND_H
#define CREATE_DATABASE_COMMAND_H
#pragma once
#include "commands/command.h"
#include "databaseManager/databaseManager.h"
#include "checks.h"
#include <iostream>
#include <string>
#include "logManager/logManager.h"

class CreateDatabaseCommand : public Command {
public:
    CreateDatabaseCommand(std::string& currentUser);
    void execute(const std::vector<std::string>& args) override;
private:
    std::string& currentUser;
};

#endif
