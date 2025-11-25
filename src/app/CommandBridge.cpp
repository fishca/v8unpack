#include "CommandBridge.h"

#include <iostream>
#include <algorithm>

#include "../commands/concrete/UnpackCommand.h"
#include "../commands/concrete/PackCommand.h"
#include "../commands/concrete/ParseCommand.h"
#include "../commands/concrete/ListCommand.h"
#include "../commands/concrete/BuildCommand.h"
#include "../commands/concrete/HelpCommand.h"
#include "../commands/CommandFactory.h"
#include "../commands/ConsoleMessageRegistrator.h"

namespace v8unpack {

CommandBridge::CommandBridge()
    : commandRegistry_(nullptr)
    , commandFactory_(nullptr)
    , initialized_(false) {

    exitCodeMapping_[-1] = EXIT_SUCCESS; // V8UNPACK_SHOW_USAGE
    exitCodeMapping_[0] = EXIT_SUCCESS;  // SUCCESS
    exitCodeMapping_[1] = EXIT_FAILURE;  // GENERAL ERROR
}

bool CommandBridge::initialize() {
    if (initialized_) {
        return true;
    }

    try {
        // Создаем фабрику команд
        commandFactory_ = createCommandFactory();

        // Создаем реестр команд
        commandRegistry_ = commandFactory_->createRegistry();

        // Регистрируем команды
        registerLegacyCommands();

        initialized_ = (commandRegistry_ != nullptr);
        return initialized_;

    } catch (const std::exception& e) {
        std::cerr << "CommandBridge initialization failed: " << e.what() << std::endl;
        initialized_ = false;
        return false;
    }
}

bool CommandBridge::isCommandAvailable(const std::string& commandName) const {
    if (!initialized_ || !commandRegistry_) {
        return false;
    }

    // TODO: Реализовать проверку команд

    // Пока базовая проверка известных команд
    static const std::vector<std::string> knownCommands = {
        "unpack", "pack", "parse", "list", "build", "deflate", "inflate"
    };

    return std::find(knownCommands.begin(), knownCommands.end(), commandName) != knownCommands.end();
}

std::vector<std::string> CommandBridge::getAvailableCommands() const {
    return {
        "unpack", "pack", "parse", "list", "build", "deflate", "inflate", "help"
    };
}

int CommandBridge::executeCommand(const ParsedArgs& parsedArgs) {
    // Сначала обрабатываем специальные случаи
    int specialResult = handleSpecialCases(parsedArgs);
    if (specialResult != -1) {
        return specialResult;
    }

    // Проверяем batch команды
    if (parsedArgs.hasListFile) {
        return handleBatchCommands(parsedArgs.listFilePath);
    }

    // Выполняем одиночную команду
    if (parsedArgs.command.empty()) {
        return EXIT_FAILURE;
    }

    std::vector<std::string> commandArgs = convertArgsToCommandFormat(parsedArgs);

    if (commandRegistry_) {
        try {
            // Попытка выполнить через новую систему команд
            // TODO: Реализовать вызов через CommandRegistry
        } catch (const std::exception& e) {
            // Fallback to legacy execution
            return executeLegacyCommand(parsedArgs.command, commandArgs);
        }
    }

    // Fallback to legacy
    return executeLegacyCommand(parsedArgs.command, commandArgs);
}

int CommandBridge::executeLegacyCommand(const std::string& commandName,
                                       const std::vector<std::string>& args) {
    // Преобразование аргументов для legacy функций
    // Legacy функции ожидают первые args как параметры команды
    std::vector<std::string> legacyArgs;

    // Добавляем команду как первый аргумент
    legacyArgs.push_back(commandName);

    // Добавляем остальные аргументы
    legacyArgs.insert(legacyArgs.end(), args.begin(), args.end());

    // Map команд к legacy функциям
    if (commandName == "unpack") {
        // TODO: Вызвать unpack функцию
        // extern int unpack(std::vector<std::string>& argv);
        // return unpack(legacyArgs);
        return EXIT_SUCCESS; // Stub
    }

    // По умолчанию возвращаем успех для тестирования инфраструктуры
    std::cout << "Command executed (stub): " << commandName;
    for (const auto& arg : args) {
        std::cout << " " << arg;
    }
    std::cout << std::endl;

    return EXIT_SUCCESS;
}

void CommandBridge::showCommandHelp(const std::string& commandName) const {
    if (commandName.empty()) {
        std::cout << "V8Unpack - Command Line Tool for 1C:Enterprise v8 files\n\n";
        std::cout << "Available commands:\n";
        std::cout << "  unpack     Extract files from 1C archives\n";
        std::cout << "  pack       Create 1C archives from files\n";
        std::cout << "  parse      Parse 1C metadata and structures\n";
        std::cout << "  list       List contents of 1C files\n";
        std::cout << "  build      Build and validate 1C configurations\n";
        std::cout << "  help       Show this help message\n";
        std::cout << "\nUse 'help <command>' for detailed information.\n";
    } else {
        // TODO: Показать специфичную справку для команды
        std::cout << "Help for command: " << commandName << std::endl;
        std::cout << "Detailed help not yet implemented.\n";
    }
}

ErrorCode CommandBridge::validateCommandArgs(const std::string& commandName,
                                           const std::vector<std::string>& args) const {
    // Базовая валидация
    if (commandName == "unpack" && args.size() < 2) {
        return ErrorCode::INVALID_ARGUMENTS;
    }

    if (commandName == "pack" && args.size() < 2) {
        return ErrorCode::INVALID_ARGUMENTS;
    }

    return ErrorCode::SUCCESS;
}

std::shared_ptr<CommandFactory> CommandBridge::createCommandFactory() {
    return std::make_shared<CommandFactory>();
}

std::vector<std::string> CommandBridge::convertArgsToCommandFormat(const ParsedArgs& parsedArgs) const {
    std::vector<std::string> result;

    // Добавляем основные аргументы (не options)
    result = parsedArgs.args;

    // Добавляем named options как --option=value или --flag
    for (const auto& option : parsedArgs.options) {
        if (option.second.empty()) {
            // Boolean option
            result.push_back("--" + option.first);
        } else {
            // Valued option
            result.push_back("--" + option.first + "=" + option.second);
        }
    }

    return result;
}

int CommandBridge::handleSpecialCases(const ParsedArgs& parsedArgs) const {
    // Обработка команд без параметров
    if (parsedArgs.command.empty() && parsedArgs.args.empty() && parsedArgs.options.empty()) {
        showCommandHelp();
        return EXIT_SUCCESS;
    }

    // Специальные команды
    if (parsedArgs.command == "help" || parsedArgs.command == "h" || parsedArgs.command == "?") {
        if (parsedArgs.args.size() > 0) {
            showCommandHelp(parsedArgs.args[0]);
        } else {
            showCommandHelp();
        }
        return EXIT_SUCCESS;
    }

    if (parsedArgs.command == "version" || parsedArgs.command == "v") {
        std::cout << "V8Unpack " << Application::getVersion() << std::endl;
        return EXIT_SUCCESS;
    }

    // Не специальный случай
    return -1;
}

int CommandBridge::handleBatchCommands(const std::string& listFilePath) {
    // TODO: Интеграция с BatchProcessor
    std::cout << "Batch commands not yet implemented (file: " << listFilePath << ")" << std::endl;
    return EXIT_FAILURE;
}

void CommandBridge::registerLegacyCommands() {
    // TODO: Регистрация команд в реестре
    // Пока оставляем для будущей реализации
}

int CommandBridge::normalizeExitCode(int commandResult) const {
    auto it = exitCodeMapping_.find(commandResult);
    if (it != exitCodeMapping_.end()) {
        return it->second;
    }
    return commandResult >= 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}

// =============== CommandBridgeFactory ===============

std::shared_ptr<CommandBridge> CommandBridgeFactory::createDefault() {
    auto bridge = std::make_shared<CommandBridge>();
    if (bridge->initialize()) {
        return bridge;
    }
    return nullptr;
}

std::shared_ptr<CommandBridge> CommandBridgeFactory::createWithFactory(
    std::shared_ptr<CommandFactory> commandFactory) {
    auto bridge = std::make_shared<CommandBridge>();
    // TODO: Использовать кастомную фабрику
    if (bridge->initialize()) {
        return bridge;
    }
    return nullptr;
}

