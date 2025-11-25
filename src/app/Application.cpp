#include "Application.h"

#include "../version.h"
#include "../V8File.h"
#include "../commands/CommandFactory.h"
#include "../commands/ConsoleMessageRegistrator.h"
#include "../metadata/RegexRegistry.h"
#include "../messageregistration.h"  // For MessageState enum
#include "LegacyFunctions.h"  // Legacy function declarations

#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <stdexcept>
#include <locale>
#include <cstdlib>  // Для EXIT_SUCCESS, EXIT_FAILURE

namespace v8unpack {

Application::Application() : initialized_(false) {
    try {
        initialize();
        initialized_ = true;
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize Application: " << e.what() << std::endl;
        initialized_ = false;
    }
}

Application::~Application() {
    // Cleanup ресурсов, если необходимо
}

int Application::run(int argc, char* argv[]) {
    if (!initialized_) {
        std::cerr << "Application not properly initialized" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        // Установка времени старта для метрик
        auto now = std::chrono::system_clock::now();
        auto time_t = std::chrono::system_clock::to_time_t(now);
        metrics_.startTime = std::ctime(&time_t);
        metrics_.startTime.erase(metrics_.startTime.find_last_not_of("\n") + 1);

        // Настройка локали
        setupLocale();

        // Парсинг аргументов
        ParsedArgs args = argumentParser_->parse(argc, argv);

        // Выполнение основной логики
        return run(args);

    } catch (const std::invalid_argument& e) {
        std::cerr << "Argument error: " << e.what() << std::endl;
        showUsage();
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

int Application::run(const ParsedArgs& args) {
    try {
        return executeCoreLogic(args);
    } catch (const ErrorInfo& error) {
        handleError(error, args);
        return EXIT_FAILURE;
    } catch (const std::exception& e) {
        ErrorInfo error(ErrorCode::UNKNOWN_ERROR, "Unexpected error", e.what());
        handleError(error, args);
        return EXIT_FAILURE;
    }
}

bool Application::isInitialized() const {
    return initialized_ &&
           argumentParser_ &&
           commandRegistry_ &&
           logger_;
}

std::string Application::getVersion() {
    return V8P_VERSION;
}

void Application::initialize() {
    // Создание основных компонентов
    argumentParser_ = std::make_shared<ArgumentParser>();
    logger_ = std::make_shared<ConsoleMessageRegistrator>();
    regexRegistry_ = std::make_shared<RegexRegistry>();

    // Создание системы команд с legacy поддержкой
    // TODO: Заменить на полную систему команд в следующих этапах
    auto factory = std::make_shared<CommandFactory>(logger_);
    commandRegistry_ = factory->createRegistry();
}

void Application::setupLocale() {
    try {
        // Аналогично setup'у в main.cpp
        std::locale::global(std::locale(""));
        std::wcout.imbue(std::locale(""));
    } catch (const std::exception& e) {
        // Локаль не критична, продолжаем выполнение
        std::cerr << "Warning: Failed to set locale: " << e.what() << std::endl;
    }
}

int Application::executeCoreLogic(const ParsedArgs& args) {
    // Обработка специальных случаев (help, version и т.д.)
    int specialResult = handleSpecialCases(args);
    if (specialResult != -1) { // -1 означает не специальный случай
        return specialResult;
    }

    // Обработка batch файлов
    if (args.hasListFile) {
        return handleBatchCommands(args.listFilePath);
    }

    // Выполнение одиночной команды
    return executeCommand(args);
}

int Application::executeCommand(const ParsedArgs& args) {
    if (args.command.empty()) {
        showUsage();
        return EXIT_FAILURE;
    }

    metrics_.commandsExecuted++;

    // Попытка выполнить через новую систему команд
    int result = executeViaCommandSystem(args.command, args.args);

    if (result == -1) { // Команда не найдена в новой системе
        // Fallback to legacy handling
        result = executeLegacyCommand(args);
    }

    return result;
}

int Application::handleBatchCommands(const std::string& listFilePath) {
    // TODO: Реализовать чтение batch файла и выполнение команд
    // Пока возвращаем not implemented
    std::cerr << "Batch command processing not yet implemented (file: " << listFilePath << ")" << std::endl;
    return EXIT_FAILURE;
}

int Application::handleSpecialCases(const ParsedArgs& args) {
    // Обработка команд, которые не требуют сложной логики
    if (args.command.empty() && args.args.empty() && args.options.empty()) {
        // Вызов без аргументов -> показать usage
        showUsage();
        return EXIT_SUCCESS;
    }

    if (args.command == "version" || args.command == "v") {
        if (logger_) {
            logger_->Status("V8Unpack " + getVersion());
        }
        return EXIT_SUCCESS;
    }

    if (args.command == "help" || args.command == "h" || args.command == "?") {
        showUsage();
        return EXIT_SUCCESS;
    }

    if (args.command == "bat") {
        // TEMPORARY: Skip legacy function call for enterprise migration
        logger_->Status("BAT command would execute (legacy function temporarily disabled)");
        return EXIT_SUCCESS;
    }

    if (args.command == "example" || args.command == "e") {
        // TEMPORARY: Skip legacy function call for enterprise migration
        logger_->Status("Example command would execute (legacy function temporarily disabled)");
        return EXIT_SUCCESS;
    }

    // Не специальный случай
    return -1;
}

int Application::executeViaCommandSystem(const std::string& commandName,
                                       const std::vector<std::string>& args) {
    // TODO: Реализовать выполнение через систему команд
    // Пока всегда возвращаем "не найдено"
    return -1;
}

std::unique_ptr<Command> Application::createLegacyCommand(const std::string& commandName) {
    // TODO: Создать wrapper'ы для legacy функций
    // Пока возвращаем nullptr
    return nullptr;
}

int Application::executeLegacyCommand(const ParsedArgs& args) {
    // Fallback to legacy execution для совместимости
    // Это позволяет медленно мигрировать команды в новую систему

    if (args.command == "unpack") {
        if (args.args.size() < 2) {
            std::cerr << "Error: unpack command requires at least 2 arguments: input_file output_dir" << std::endl;
            return EXIT_FAILURE;
        }
        std::string filename = args.args[0];
        std::string dirname = args.args[1];
        std::string block_name = (args.args.size() > 2) ? args.args[2] : "";
        int result = UnpackToFolder(filename, dirname, block_name, true);
        if (result != 0) {
            std::cerr << "Unpack failed with code: " << result << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (args.command == "pack") {
        if (args.args.size() < 2) {
            std::cerr << "Error: pack command requires at least 2 arguments: input_dir output_file" << std::endl;
            return EXIT_FAILURE;
        }
        std::string dirname = args.args[0];
        std::string filename = args.args[1];
        int result = PackFromFolder(dirname, filename);
        if (result != 0) {
            std::cerr << "Pack failed with code: " << result << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (args.command == "parse") {
        if (args.args.size() < 2) {
            std::cerr << "Error: parse command requires at least 2 arguments: input_file output_dir" << std::endl;
            return EXIT_FAILURE;
        }
        std::string filename = args.args[0];
        std::string dirname = args.args[1];
        std::vector<std::string> filter(args.args.begin() + 2, args.args.end());
        int result = Parse(filename, dirname, filter);
        if (result != 0) {
            std::cerr << "Parse failed with code: " << result << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (args.command == "build") {
        if (args.args.size() < 2) {
            std::cerr << "Error: build command requires at least 2 arguments: input_dir output_file" << std::endl;
            return EXIT_FAILURE;
        }
        std::string dirname = args.args[0];
        std::string filename = args.args[1];
        bool dont_deflate = args.hasOption("nopack");
        int result = BuildCfFile(dirname, filename, !dont_deflate);
        if (result != 0) {
            std::cerr << "Build failed with code: " << result << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (args.command == "lf" || args.command == "listfiles") {
        if (args.args.size() < 1) {
            std::cerr << "Error: listfiles command requires 1 argument: input_file" << std::endl;
            return EXIT_FAILURE;
        }
        std::string filename = args.args[0];
        int result = ListFiles(filename);
        if (result != 0) {
            std::cerr << "ListFiles failed with code: " << result << std::endl;
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    // Для других команд - показываем что получили команду
    if (logger_) {
        std::ostringstream oss;
        oss << "Executing legacy command: " << args.command;
        if (!args.args.empty()) {
            oss << " with args: ";
            for (size_t i = 0; i < args.args.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << args.args[i];
            }
        }
        logger_->Status(oss.str());
    }

    // TODO: Здесь добавить mapping других команд к legacy функциям
    // Сейчас возвращаем success для тестирования инфраструктуры
    return EXIT_SUCCESS;
}

void Application::handleError(const ErrorInfo& error, const ParsedArgs& args) {
    metrics_.errorsEncountered++;

    // Логирование ошибки
    if (logger_) {
        std::ostringstream oss;
        oss << "Error [" << static_cast<int>(error.code()) << "]: " << error.message();
        if (!error.details().empty()) {
            oss << " - " << error.details();
        }

        logger_->AddMessage(oss.str(), MessageState::Error);
    }

    // Вывод в stderr для непосредственной видимости
    std::cerr << "Error: " << error.message();
    if (!error.details().empty()) {
        std::cerr << " (" << error.details() << ")";
    }
    std::cerr << std::endl;

    // Показ дополнительной информации при необходимости
    if (error.isFileError() && !args.originalCommandLine.empty()) {
        std::cerr << "Command was: " << args.originalCommandLine << std::endl;
    }
}

void Application::showUsage() {
    std::cout << "V8Upack Version " << V8P_VERSION << " Copyright (c)" << V8P_RIGHT << std::endl;
    std::cout << std::endl;
    std::cout << "Unpack, pack, deflate and inflate 1C v8 file (*.cf)" << std::endl;
    std::cout << std::endl;
    std::cout << "V8UNPACK" << std::endl;
    std::cout << "  -U[NPACK]            in_filename.cf     out_dirname [block_name]" << std::endl;
    std::cout << "  -U[NPACK]  -L[IST]   listfile" << std::endl;
    std::cout << "  -PA[CK]              in_dirname         out_filename.cf" << std::endl;
    std::cout << "  -PA[CK]    -L[IST]   listfile" << std::endl;
    std::cout << "  -I[NFLATE]           in_filename.data   out_filename" << std::endl;
    std::cout << "  -I[NFLATE] -L[IST]   listfile" << std::endl;
    std::cout << "  -D[EFLATE]           in_filename        filename.data" << std::endl;
    std::cout << "  -D[EFLATE] -L[IST]   listfile" << std::endl;
    std::cout << "  -P[ARSE]             in_filename        out_dirname [block_name1 block_name2 ...]" << std::endl;
    std::cout << "  -P[ARSE]   -L[IST]   listfile" << std::endl;
    std::cout << "  -B[UILD] [-N[OPACK]] in_dirname         out_filename" << std::endl;
    std::cout << "  -B[UILD] [-N[OPACK]] -L[IST] listfile" << std::endl;
    std::cout << "  -L[IST]              listfile" << std::endl;
    std::cout << "  -LISTFILES|-LF       in_filename" << std::endl;
    std::cout << "  -E[XAMPLE]" << std::endl;
    std::cout << "  -BAT" << std::endl;
    std::cout << "  -V[ERSION]" << std::endl;
}

// ===== Реализация ApplicationFactory =====

std::unique_ptr<Application> ApplicationFactory::createDefault() {
    return std::make_unique<Application>();
}

std::unique_ptr<Application> ApplicationFactory::createWithComponents(
    std::shared_ptr<ArgumentParser> argParser,
    std::shared_ptr<CommandRegistry> cmdRegistry,
    std::shared_ptr<MessageRegistrator> logger) {

    // TODO: Реализовать создание Application с кастомными компонентами
    // Пока возвращаем дефолтный инстанс
    return createDefault();
}

} // namespace v8unpack
