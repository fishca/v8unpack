#ifndef APP_ARGUMENT_PARSER_H
#define APP_ARGUMENT_PARSER_H

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace v8unpack {

/**
 * @brief Структура для представления распарсенных аргументов командной строки
 */
struct ParsedArgs {
    /**
     * @brief Команда для выполнения (например, "unpack", "pack", "parse")
     */
    std::string command;

    /**
     * @brief Позиционные аргументы без ключей
     * Для unpack: ["input_file.cf", "output_dir/"]
     * Для pack: ["input_dir/", "output_file.cf"]
     */
    std::vector<std::string> args;

    /**
     * @brief Опции командной строки с ключами (--key=value или --flag)
     * Поддерживает: --key value, --key=value, --flag
     */
    std::map<std::string, std::string> options;

    /**
     * @brief Флаг, указывающий что есть файл с списком команд
     */
    bool hasListFile = false;

    /**
     * @brief Путь к файлу со списком команд (если hasListFile=true)
     */
    std::string listFilePath;

    /**
     * @brief Оригинальная командная строка для вывода справки
     */
    std::string originalCommandLine;

    // Вспомогательные методы (inline реализации)
    bool hasOption(const std::string& key) const {
        return options.find(key) != options.end();
    }

    std::string getOption(const std::string& key, const std::string& defaultValue = "") const {
        auto it = options.find(key);
        return (it != options.end()) ? it->second : defaultValue;
    }

    size_t argCount() const {
        return args.size();
    }

    std::string getArg(size_t index) const {
        return (index < args.size()) ? args[index] : std::string();
    }

    // Проверка на пустоту результата
    bool empty() const {
        return command.empty() && args.empty() && options.empty();
    }
};

/**
 * @brief Парсер аргументов командной строки для V8Unpack
 *
 * Отвечает за разбор и валидацию аргументов командной строки.
 * Преобразует массив argv в структурированную информацию ParsedArgs.
 */
class ArgumentParser {
public:
    /**
     * @brief Конструктор
     */
    ArgumentParser();

    /**
     * @brief Деструктор
     */
    ~ArgumentParser() = default;

    /**
     * @brief Парсинг аргументов командной строки
     *
     * @param argc Количество аргументов (из main)
     * @param argv Массив аргументов (из main)
     * @return ParsedArgs с распарсенными данными
     *
     * @throws std::invalid_argument если аргументы некорректны
     */
    ParsedArgs parse(int argc, char* argv[]);

    /**
     * @brief Перегрузка для вектора строк (для тестирования)
     *
     * @param args Вектор аргументов
     * @return ParsedArgs с распарсенными данными
     */
    ParsedArgs parse(const std::vector<std::string>& args);

private:
    /**
     * @brief Основная логика парсинга
     */
    ParsedArgs parseInternal(const std::vector<std::string>& args);

    /**
     * @brief Валидация распарсенных аргументов
     *
     * @param parsed Распарсенные аргументы для проверки
     * @throws std::invalid_argument если валидация не прошла
     */
    void validateArgs(const ParsedArgs& parsed);

    /**
     * @brief Определение команды из первого аргумента
     *
     * @param firstArg Первый аргумент (потенциальная команда)
     * @return Нормализованная команда или пустая строка
     */
    std::string extractCommand(const std::string& firstArg);

    /**
     * @brief Парсинг опций вида --key[=value]
     *
     * @param arg Аргумент для парсинга
     * @return pair<key, value> или empty если не опция
     */
    std::pair<std::string, std::string> parseOption(const std::string& arg);

    /**
     * @brief Проверка является ли аргумент опцией
     *
     * @param arg Аргумент для проверки
     * @return true если начинается с "--"
     */
    bool isOption(const std::string& arg) const;

    /**
     * @brief Проверка известных команд V8Unpack
     *
     * @param command Команда для проверки
     * @return true если команда распознана
     */
    bool isValidCommand(const std::string& command) const;

    /**
     * @brief Получение списка всех валидных команд
     */
    std::vector<std::string> getValidCommands() const;

private:
    // Константы для парсинга
    static const char OPTION_PREFIX[];
    static const char OPTION_SEPARATOR;
};

/**
 * @brief Утилиты для работы с аргументами командной строки
 */
class ArgumentUtils {
public:
    /**
     * @brief Преобразование argc/argv в вектор строк
     */
    static std::vector<std::string> argvToVector(int argc, char* argv[]);

    /**
     * @brief ПреобразованиеParsedArgs обратно в командную строку
     */
    static std::string argsToString(const ParsedArgs& args);

    /**
     * @brief Валидация что все требуемые аргументы присутствуют
     */
    static bool validateRequiredArgs(const ParsedArgs& args,
                                   const std::vector<std::string>& requiredOptions = {},
                                   size_t minPositionalArgs = 0);
};

} // namespace v8unpack

#endif // APP_ARGUMENT_PARSER_H
