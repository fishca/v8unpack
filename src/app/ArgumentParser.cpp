#include "ArgumentParser.h"

#include <algorithm>
#include <sstream>

namespace v8unpack {

ArgumentParser::ArgumentParser() {
    // Конструктор пока пустой, может использоваться для инициализации
}

// Определение static констант вне класса
const char ArgumentParser::OPTION_PREFIX[] = "--";
const char ArgumentParser::OPTION_SEPARATOR = '=';

ParsedArgs ArgumentParser::parse(int argc, char* argv[]) {
    std::vector<std::string> args;
    args.reserve(argc);

    for (int i = 0; i < argc; ++i) {
        if (argv[i]) {
            args.emplace_back(argv[i]);
        }
    }

    return parse(args);
}

ParsedArgs ArgumentParser::parse(const std::vector<std::string>& args) {
    // Сохраняем оригинальную командную строку для справки
    std::stringstream cmdStream;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0) cmdStream << " ";
        cmdStream << args[i];
    }

    ParsedArgs result = parseInternal(args);
    result.originalCommandLine = cmdStream.str();

    // Валидируем результат
    validateArgs(result);

    return result;
}

ParsedArgs ArgumentParser::parseInternal(const std::vector<std::string>& args) {
    ParsedArgs result;

    if (args.size() < 2) { // Минимум: program_name command
        return result; // Пустой результат для справки
    }

    // Пропускаем имя программы (args[0]) и начинаем с команды (args[1])
    size_t currentIndex = 1;

    // Первый аргумент - команда
    if (currentIndex < args.size()) {
        result.command = extractCommand(args[currentIndex]);
        currentIndex++;
    }

    // Разбираем остальные аргументы
    for (; currentIndex < args.size(); ++currentIndex) {
        const std::string& arg = args[currentIndex];

        if (isOption(arg)) {
            // Обработка опций
            auto optionPair = parseOption(arg);
            if (!optionPair.first.empty()) {
                std::string key = optionPair.first;
                std::string value = optionPair.second;

                // Специальная обработка для list файлов
                if (key == "list" || key == "l") {
                    result.hasListFile = true;
                    if (!value.empty()) {
                        result.listFilePath = value;
                    } else if (currentIndex + 1 < args.size()) {
                        // Значение может быть в следующем аргументе
                        result.listFilePath = args[++currentIndex];
                    }
                } else {
                    result.options[key] = value;
                }
            }
        } else {
            // Позиционный аргумент
            result.args.push_back(arg);
        }
    }

    return result;
}

void ArgumentParser::validateArgs(const ParsedArgs& parsed) {
    // Базовая валидация
    if (!parsed.command.empty() && !isValidCommand(parsed.command)) {
        throw std::invalid_argument("Unknown command: " + parsed.command);
    }

    // Проверка специальных случаев
    if (parsed.hasListFile && parsed.listFilePath.empty()) {
        throw std::invalid_argument("List file path is required when using --list option");
    }

    // Валидация командно-специфичных требований может быть добавлена позже
    // Например, определенные команды требуют минимального количества аргументов
}

std::string ArgumentParser::extractCommand(const std::string& firstArg) {
    if (firstArg.empty()) {
        return "";
    }

    std::string command = firstArg;

    // Нормализация - убираем возможные префиксы и приводим к нижнему регистру
    if (command.size() > 1 && command[0] == '-') {
        // Убираем один или два дефиса в начале
        size_t startPos = (command.size() > 1 && command[1] == '-') ? 2 : 1;
        command = command.substr(startPos);
    }

    // Приводим к нижнему регистру для case-insensitive matching
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    return command;
}

std::pair<std::string, std::string> ArgumentParser::parseOption(const std::string& arg) {
    if (!isOption(arg)) {
        return std::make_pair("", "");
    }

    // Убираем префикс "--"
    std::string option = arg.substr(2);

    // Ищем разделитель "="
    size_t equalPos = option.find(OPTION_SEPARATOR);
    if (equalPos != std::string::npos) {
        // Форма --key=value
        std::string key = option.substr(0, equalPos);
        std::string value = option.substr(equalPos + 1);
        return std::make_pair(key, value);
    } else {
        // Форма --flag (или --key без значения)
        return std::make_pair(option, "");
    }
}

bool ArgumentParser::isOption(const std::string& arg) const {
    return arg.size() >= 2 && arg.substr(0, 2) == OPTION_PREFIX;
}

bool ArgumentParser::isValidCommand(const std::string& command) const {
    static const std::vector<std::string> validCommands = getValidCommands();
    return std::find(validCommands.begin(), validCommands.end(), command) != validCommands.end();
}

std::vector<std::string> ArgumentParser::getValidCommands() const {
    return {
        // Основные операции
        "unpack", "pack",

        // Сжатие/распаковка
        "deflate", "inflate", "und", "undeflate",

        // Парсинг
        "parse", "parsestring", "parsemetadata", "save", "savefiles", "listfiles", "lf",

        // Сборка
        "build",

        // Списки команд
        "list", "l", "process_list",

        // Диагностика
        "version", "v", "test", "t", "usage",

        // Справка
        "help", "h", "?" , "bat", "example", "e"
    };
}

// =========== Реализация ArgumentUtils ===========

std::vector<std::string> ArgumentUtils::argvToVector(int argc, char* argv[]) {
    std::vector<std::string> result;
    result.reserve(argc);

    for (int i = 0; i < argc; ++i) {
        if (argv[i]) {
            result.emplace_back(argv[i]);
        }
    }

    return result;
}

std::string ArgumentUtils::argsToString(const ParsedArgs& args) {
    std::stringstream ss;

    // Команда
    if (!args.command.empty()) {
        ss << args.command;
    }

    // Позиционные аргументы
    for (const auto& arg : args.args) {
        ss << " " << arg;
    }

    // Опции
    for (const auto& option : args.options) {
        ss << " --" << option.first;
        if (!option.second.empty()) {
            ss << "=" << option.second;
        }
    }

    // List файл
    if (args.hasListFile && !args.listFilePath.empty()) {
        ss << " --list " << args.listFilePath;
    }

    return ss.str();
}

bool ArgumentUtils::validateRequiredArgs(const ParsedArgs& args,
                                       const std::vector<std::string>& requiredOptions,
                                       size_t minPositionalArgs) {
    // Проверка минимального количества позиционных аргументов
    if (args.argCount() < minPositionalArgs) {
        return false;
    }

    // Проверка обязательных опций
    for (const auto& option : requiredOptions) {
        if (!args.hasOption(option)) {
            return false;
        }
    }

    return true;
}

} // namespace v8unpack
