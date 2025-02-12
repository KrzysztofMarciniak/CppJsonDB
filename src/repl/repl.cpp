#include "headers/repl.h"
#include "utils.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <readline/readline.h>
#include <readline/history.h>
#include <cctype>
#include <iterator>
#include <limits>
#include <regex>
#include <chrono>
#include <fstream>

Repl::Repl(CommandFactory& factory, std::string& currentUser, std::string& currentDatabase)
    : commandFactory(factory), currentUser(currentUser), currentDatabase(currentDatabase) {
}

void Repl::processStartupCommands(const std::vector<std::string>& args) {
    std::string commandLine = joinTokens(args, 0, args.size());

    std::string delimiter = "//";
    size_t pos = 0;
    std::string commandSegment;
    while ((pos = commandLine.find(delimiter)) != std::string::npos) {
        commandSegment = commandLine.substr(0, pos);
        commandLine.erase(0, pos + delimiter.length());

        commandSegment.erase(commandSegment.begin(), std::find_if(commandSegment.begin(), commandSegment.end(), [](unsigned char ch) { return !std::isspace(ch); }));
        commandSegment.erase(std::find_if(commandSegment.rbegin(), commandSegment.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), commandSegment.end());

        if (!commandSegment.empty()) {
            processCommand(commandSegment);
        }
    }

    commandLine.erase(commandLine.begin(), std::find_if(commandLine.begin(), commandLine.end(), [](unsigned char ch) { return !std::isspace(ch); }));
    commandLine.erase(std::find_if(commandLine.rbegin(), commandLine.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), commandLine.end());

    if (!commandLine.empty()) {
        processCommand(commandLine);
    }
}

void Repl::start() {
    char* input;
    while ((input = readline(getPrompt().c_str())) != nullptr) {
        if (input[0] != '\0') {
            add_history(input);
        }

        std::stringstream ss(input);
        std::string commandLine;
        while (std::getline(ss, commandLine, ';')) {
            commandLine.erase(commandLine.begin(), std::find_if(commandLine.begin(), commandLine.end(), [](unsigned char ch) { return !std::isspace(ch); }));
            commandLine.erase(std::find_if(commandLine.rbegin(), commandLine.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), commandLine.end());

            if (!commandLine.empty()) {
                processCommand(commandLine);
            }
        }

        free(input);
    }
}

void Repl::processCommand(const std::string& input) {
    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::string> tokens = tokenize(input);
    if (tokens.empty()) {
        std::cout << "No command entered." << std::endl;
        return;
    }

    std::string commandName;
    std::vector<std::string> args;

    for (size_t i = 0; i < tokens.size(); ++i) {
        std::string possibleCommand = joinTokens(tokens, 0, i + 1);
        if (commandFactory.getCommand(possibleCommand)) {
            commandName = possibleCommand;
            args = std::vector<std::string>(tokens.begin() + i + 1, tokens.end());
            break;
        }
    }

    if (commandName.empty()) {
        suggestCommands(tokens[0]);
    } else {
        auto command = commandFactory.getCommand(commandName);
        if (command) {
            command->execute(args);
        } else {
            std::cout << "Command not found: " << commandName << std::endl;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    if (saveExecutionDetails) {
        saveExecutionDetailsToFile(commandName, duration);
    }
}

void Repl::saveExecutionDetailsToFile(const std::string& commandName, std::chrono::duration<double> duration) {
    std::ofstream outFile("execution_details.log", std::ios_base::app);
    if (outFile.is_open()) {
        outFile << "Command: " << commandName << " executed in " << duration.count() << " seconds." << std::endl;
        outFile.close();
    } else {
        std::cerr << "Failed to open execution details log file." << std::endl;
    }
}

std::vector<std::string> Repl::tokenize(const std::string& input) {
    std::vector<std::string> tokens;
    std::regex tokenRegex(R"(([^()\s,]+)|(\([^)]*\))|(\s*".*?")|(\s*'.*?'))");
    auto wordsBegin = std::sregex_iterator(input.begin(), input.end(), tokenRegex);
    auto wordsEnd = std::sregex_iterator();

    for (std::sregex_iterator i = wordsBegin; i != wordsEnd; ++i) {
        std::smatch match = *i;
        tokens.push_back(match.str());
    }

    return tokens;
}

std::string Repl::getPrompt() const {
    std::string userPrompt = getUserPrompt();
    std::string databasePrompt = getDatabasePrompt();
    return buildPrompt(userPrompt, databasePrompt);
}

std::string Repl::getUserPrompt() const {
    std::string prompt;
    if (!currentUser.empty()) {
        prompt += "\033[1;32m[" + currentUser + "]\033[0m";
    }
    return prompt;
}

std::string Repl::getDatabasePrompt() const {
    std::string prompt;
    if (!currentDatabase.empty()) {
        if (!currentUser.empty()) {
            prompt += "::";
        }
        prompt += "\033[1;34m(" + currentDatabase + ")\033[0m";
    }
    return prompt;
}

std::string Repl::buildPrompt(const std::string& userPrompt, const std::string& databasePrompt) const {
    std::string prompt = userPrompt;
    prompt += databasePrompt;
    prompt += "> ";
    return prompt;
}

std::string Repl::joinTokens(const std::vector<std::string>& tokens, size_t start, size_t end) const {
    std::ostringstream oss;
    for (size_t i = start; i < end; ++i) {
        if (i != start) {
            oss << " ";
        }
        oss << tokens[i];
    }
    return oss.str();
}

void Repl::setSaveExecutionDetails(bool saveDetails) {
    saveExecutionDetails = saveDetails;
}
void Repl::suggestCommands(const std::string& input) {
    std::vector<std::string> availableCommands = commandFactory.getAllCommands();

    std::vector<std::string> suggestions;

    for (const auto& command : availableCommands) {
        if (command.find(input) == 0) {
            suggestions.push_back(command);
        }
    }

    if (!suggestions.empty()) {
        std::cout << "Did you mean: " << std::endl;
        for (const auto& suggestion : suggestions) {
            std::cout << "  - " << suggestion << std::endl;
        }
    } else {
        std::cout << "No command suggestions found for: " << input << std::endl;
    }
}
