#ifndef APP_APPLICATION_H
#define APP_APPLICATION_H

#include <memory>
#include <string>
#include <vector>

// Forward declarations и локальные headers
#include "ArgumentParser.h"
#include "../utils/ErrorCodes.h"

// Нужны полные определения для членов класса
#include "../commands/CommandRegistry.h"
#include "../commands/CommandFactory.h"
#include "../commands/ConsoleMessageRegistrator.h"
#include "../metadata/RegexRegistry.h"

// Using declarations для упрощения
namespace v8unpack {

using ErrorInfo = ::v8unpack::ErrorInfo;
using ParsedArgs = ::v8unpack::ParsedArgs;
using ArgumentParser = ::v8unpack::ArgumentParser;
using CommandRegistry = ::v8unpack::CommandRegistry;
using RegexRegistry = ::v8unpack::RegexRegistry;
using Command = ::v8unpack::Command;

/**
 * @brief Главный класс приложения V8Unpack
 *
 * Отвечает за координацию всех компонентов приложения.
 * Реализует основной жизненный цикл: от парсинга аргументов до выполнения команд.
 *
 * Паттерн: Facade - предоставляет простой интерфейс для сложной системы.
 */
class Application {
public:
    /**
     * @brief Конструктор - инициализация всех компонентов
     */
    Application();

    /**
     * @brief Деструктор
     */
    ~Application();

    /**
     * @brief Основной метод выполнения приложения
     *
     * Выполняет полный жизненный цикл:
     * 1. Парсинг аргументов
     * 2. Настройка локали и logging
     * 3. Выполнение команды или batch обработки
     * 4. Обработка ошибок и выход
     *
     * @param argc Количество аргументов (из main)
     * @param argv Массив аргументов (из main)
     * @return Код возврата (0 - успех, >0 - ошибка)
     */
    int run(int argc, char* argv[]);

    /**
     * @brief Выполнение с предварительно распарсенными аргументами
     *
     * Удобно для тестирования - позволяет обойти парсинг командной строки.
     *
     * @param args Распарсенные аргументы
     * @return Код возврата
     */
    int run(const ParsedArgs& args);

    /**
     * @brief Проверка что приложение инициализировано корректно
     *
     * @return true если все компоненты готовы к работе
     */
    bool isInitialized() const;

    /**
     * @brief Получение версии приложения
     */
    static std::string getVersion();

private:
    /**
     * @brief Инициализация базовых компонентов
     */
    void initialize();

    /**
     * @brief Настройка локали приложения
     */
    void setupLocale();

    /**
     * @brief Выполнение основной бизнес-логики
     *
     * @param args Распарсенные аргументы
     * @return Код возврата
     */
    int executeCoreLogic(const ParsedArgs& args);

    /**
     * @brief Обработка одиночной команды
     *
     * @param args Аргументы команды
     * @return Код возврата (0 - успех, >0 - ошибка)
     */
    int executeCommand(const ParsedArgs& args);

    /**
     * @brief Обработка batch файла с командами
     *
     * @param listFilePath Путь к файлу со списком команд
     * @return Код возврата
     */
    int handleBatchCommands(const std::string& listFilePath);

    /**
     * @brief Обработка специальных случаев (help, version и т.д.)
     *
     * @param args Аргументы
     * @return Код возврата (или -1 если не специальный случай)
     */
    int handleSpecialCases(const ParsedArgs& args);

    /**
     * @brief Выполнение команды через систему команд
     *
     * @param commandName Имя команды
     * @param args Аргументы команды
     * @return Код возврата
     */
    int executeViaCommandSystem(const std::string& commandName,
                               const std::vector<std::string>& args);

    /**
     * @brief Создание legacy command wrapper
     *
     * Обертывает старые глобальные функции в новый интерфейс команд.
     *
     * @param commandName Имя команды
     * @return unique_ptr к командному объекту или nullptr
     */
    std::unique_ptr<class Command> createLegacyCommand(const std::string& commandName);

    /**
     * @brief Выполнение legacy команды через fallback механизм
     *
     * @param args Распарсенные аргументы команды
     * @return Код возврата
     */
    int executeLegacyCommand(const ParsedArgs& args);

    /**
     * @brief Обработка и логирование ошибок
     *
     * @param error Информация об ошибке
     * @param args Исходные аргументы для дополнительного контекста
     */
    void handleError(const ErrorInfo& error, const ParsedArgs& args = ParsedArgs());

    /**
     * @brief Показ справки о программе
     */
    void showUsage();

private:
    // Основные компоненты приложения (Dependency Injection ready)
    std::shared_ptr<ArgumentParser> argumentParser_;
    std::shared_ptr<CommandRegistry> commandRegistry_;
    std::shared_ptr<MessageRegistrator> logger_;
    std::shared_ptr<RegexRegistry> regexRegistry_;

    // Флаг инициализации
    bool initialized_;

    // Метрики выполнения (для отладки и мониторинга)
    struct ExecutionMetrics {
        size_t commandsExecuted = 0;
        size_t errorsEncountered = 0;
        std::string startTime;
    } metrics_;
};

/**
 * @brief Фабрика для создания экземпляров Application
 *
 * Позволяет создавать Application с различными конфигурациями,
 * полезно для тестирования или создания кастомных версий.
 */
class ApplicationFactory {
public:
    /**
     * @brief Создание стандартного приложения
     */
    static std::unique_ptr<Application> createDefault();

    /**
     * @brief Создание приложения с кастомными компонентами
     *
     * @param argParser Кастомный парсер аргументов
     * @param cmdRegistry Кастомный реестр команд
     * @param logger Кастомный логгер
     */
    static std::unique_ptr<Application> createWithComponents(
        std::shared_ptr<ArgumentParser> argParser,
        std::shared_ptr<CommandRegistry> cmdRegistry,
        std::shared_ptr<::MessageRegistrator> logger);
};

} // namespace v8unpack

#endif // APP_APPLICATION_H
