#include "CommandRegistry.h"
#include "../messageregistration.h"
#include <algorithm>
#include <iostream>
#include <sstream>

namespace v8unpack {

CommandRegistry::CommandRegistry(std::shared_ptr<::MessageRegistrator> logger)
    : logger_(logger) {
}

void CommandRegistry::registerCommand(std::unique_ptr<Command> command) {
    if (!command) {
        // TODO: Log error when logger is available
        return;
    }

    std::string commandName = command->getName();
    if (commands_.find(commandName) != commands_.end()) {
        // TODO: Log error when logger is available
        return;
    }

    commands_[commandName] = std::move(command);

    // TODO: Log success when logger is available
}

Command* CommandRegistry::getCommandInternal(const std::string& name) const {
    auto it = commands_.find(name);
    if (it != commands_.end()) {
        return it->second.get();
    }
    return nullptr;
}

Command* CommandRegistry::getCommand(const std::string& name) const {
    if (name.empty()) {
        return nullptr;
    }

    std::string processedName = name;

    // Убираем префикс '-' если он есть
    if (!processedName.empty() && processedName[0] == '-') {
        processedName = processedName.substr(1);
    }

    // Проверяем полное имя
    Command* command = getCommandInternal(processedName);
    if (command) {
        return command;
    }

    // Если не найдено, пытаемся найти команду по первой букве (алиас)
    if (processedName.length() == 1) {
        for (const auto& pair : commands_) {
            if (!pair.first.empty() && pair.first[0] == processedName[0]) {
                return pair.second.get();
            }
        }
    }

    return nullptr;
}

std::vector<std::string> CommandRegistry::getAvailableCommands() const {
    std::vector<std::string> commandNames;
    commandNames.reserve(commands_.size());

    for (const auto& pair : commands_) {
        commandNames.push_back(pair.first);
    }

    // Сортируем для предсказуемости
    std::sort(commandNames.begin(), commandNames.end());

    return commandNames;
}

bool CommandRegistry::hasCommand(const std::string& name) const {
    return getCommand(name) != nullptr;
}

void CommandRegistry::showHelp() const {
    std::cout << "Available commands:" << std::endl;
    std::cout << "==================" << std::endl;

    auto commandNames = getAvailableCommands();
    for (const auto& name : commandNames) {
        Command* cmd = getCommandInternal(name);
        if (cmd) {
            std::cout << "  " << name << " - " << cmd->getDescription() << std::endl;
        }
    }

    std::cout << std::endl;
    std::cout << "Use 'v8unpack help <command>' for detailed help on a specific command." << std::endl;
    std::cout << "Commands can be abbreviated to their first letter (e.g., 'u' for 'unpack')." << std::endl;
}

void CommandRegistry::showCommandHelp(const std::string& commandName) const {
    Command* command = getCommand(commandName);
    if (!command) {
        std::cout << "Command '" << commandName << "' not found." << std::endl;
        std::cout << "Use 'v8unpack help' to see available commands." << std::endl;
        return;
    }

    std::cout << "Command: " << command->getName() << std::endl;
    std::cout << "Description: " << command->getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    command->showUsage();
}

size_t CommandRegistry::getCommandCount() const {
    return commands_.size();
}

int CommandRegistry::executeCommand(const std::string& commandName, const std::vector<std::string>& args) {
    Command* command = getCommand(commandName);
    if (!command) {
        if (logger_) {
            logger_->AddError("Command not found", "executeCommand", "Command '" + commandName + "' is not registered");
        }
        return -1; // Command not found
    }

    try {
        return command->execute(args);
    } catch (const std::exception& e) {
        if (logger_) {
            logger_->AddError("Command execution failed", "executeCommand", std::string("Command '") + commandName + "' failed: " + e.what());
        }
        return -2; // Execution failed
    } catch (...) {
        if (logger_) {
            logger_->AddError("Command execution failed", "executeCommand", std::string("Command '") + commandName + "' failed with unknown error");
        }
        return -2; // Execution failed
    }
}

} // namespace v8unpack
