#ifndef REPL_H
#define REPL_H

#include <iostream>
#include <sstream>
#include <vector>
#include <memory>
#include <chrono>
#include "commandFactory/headers/commandFactory.h"
#include <readline/readline.h>
#include <readline/history.h>
#include <unordered_set>
#include "utils.h"

extern const bool SAVE_EXECUTION_DETAILS;

class Repl {
public:
    Repl(CommandFactory& factory, std::string& currentUser, std::string& currentDatabase);
    void start();
    void suggestCommands(const std::string& input);
    void processStartupCommands(const std::vector<std::string>& args);
    void setSaveExecutionDetails(bool saveDetails);

private:
    CommandFactory& commandFactory;
    std::string& currentUser;
    std::string& currentDatabase;
    bool saveExecutionDetails = false;
    void processCommand(const std::string& input);
    std::vector<std::string> tokenize(const std::string& input);
    std::string getPrompt() const;
    std::string getUserPrompt() const;
    std::string getDatabasePrompt() const;
    std::string buildPrompt(const std::string& userPrompt, const std::string& databasePrompt) const;
    std::string joinTokens(const std::vector<std::string>& tokens, size_t start, size_t end) const;
    void saveExecutionDetailsToFile(const std::string& commandName, std::chrono::duration<double> duration);
};

#endif
