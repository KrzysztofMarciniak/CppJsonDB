#ifndef COMMAND_H
#define COMMAND_H

#include <vector>
#include <string>

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(const std::vector<std::string>& args) = 0;
};

#endif
