#ifndef LIST_DATABASES_COMMAND_H
#define LIST_DATABASES_COMMAND_H
#pragma once
#include "commands/command.h"
#include "databaseManager/databaseManager.h"
#include <iostream>
#include <fstream>

class ListDatabasesCommand : public Command {
public:
    void execute(const std::vector<std::string>& args) override;
};

#endif
