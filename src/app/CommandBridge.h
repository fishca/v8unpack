#ifndef APP_COMMAND_BRIDGE_H
#define APP_COMMAND_BRIDGE_H

#include <memory>
#include <string>
#include <vector>
#include <map>

#include "ArgumentParser.h"
#include "../utils/ErrorCodes.h"

namespace v8unpack {

class CommandRegistry;
class CommandFactory;

/**
 * @brief Мост между новой архитектурой Application и существующей системой команд
 *
 * CommandBridge связывает:
 * - ArgumentParser (новая инфраструктура)
 * - CommandRegistry и CommandFactory (существующая система команд)
 * - Application (координатор)
 *
 * Позволяет постепенно интегрировать новые команды в архитектуру
 * без нарушения обратной совместимости.
 */
class CommandBridge {
public:
    /**
     * @brief Конструктор
     */
    CommandBridge();

    /**
     * @brief Деструктор
     */
    ~CommandBridge() = default;

    /**
     * @brief Инициализация моста команд
     *
     * @return true если инициализация успешна
     */
    bool initialize();

    /**
     * @brief Проверка доступности команды
     *
     * @param commandName Имя команды
     * @return true если команда доступна для выполнения
     */
    bool isCommandAvailable(const std::string& commandName) const;

    /**
     * @brief Получение списка доступных команд
     *
     * @return Вектор имен доступных команд
     */
    std::vector<std::string> getAvailableCommands() const;

    /**
     * @brief Выполнение команды через мост
     *
     * @param parsedArgs Распаренные аргументы из ArgumentParser
     * @return Код возврата выполнения команды
     */
    int executeCommand(const ParsedArgs& parsedArgs);

    /**
     * @brief Показ справки для команды
     *
     * @param commandName Имя команды (или пустая строка для общего справки)
     */
    void showCommandHelp(const std::string& commandName = "") const;

    /**
     * @brief Валидация аргументов для команды
     *
     * @param commandName Имя команды
     * @param args Аргументы команды
     * @return Код ошибки валидации
     */
    ErrorCode validateCommandArgs(const std::string& commandName,
                                const std::vector<std::string>& args) const;

private:
    /**
     * @brief Создание фабрики команд
     *
     * @return Указатель на фабрику команд
     */
    std::shared_ptr<CommandFactory> createCommandFactory();

    /**
     * @brief Преобразование ParsedArgs в аргументы для команды
     *
     * @param parsedArgs Распаренные аргументы
     * @return Вектор строковых аргументов для команды
     */
    std::vector<std::string> convertArgsToCommandFormat(const ParsedArgs& parsedArgs) const;

    /**
     * @brief Обработка специальных случаев (help, version и т.д.)
     *
     * @param parsedArgs Распаренные аргументы
     * @return Код возврата специального случая или -1 если не специальный
     */
    int handleSpecialCases(const ParsedArgs& parsedArgs) const;

    /**
     * @brief Обработка batch команд через BatchProcessor
     *
     * @param listFilePath Путь к файлу со списком команд
     * @return Код возврата
     */
    int handleBatchCommands(const std::string& listFilePath);

    /**
     * @brief Регистрация legacy команд для обратной совместимости
     *
     * Подключает существующие глобальные функции команд
     * к новым интерфейсам для постепенной миграции.
     */
    void registerLegacyCommands();

    /**
     * @brief Выполнение команды через legacy функции
     *
     * @param commandName Имя команды
     * @param args Аргументы команды
     * @return Код возврата
     */
    int executeLegacyCommand(const std::string& commandName,
                           const std::vector<std::string>& args);

    /**
     * @brief Преобразование кода возврата команды в стандартный exit code
     *
     * @param commandResult Код возврата команды (legacy стиль)
     * @return Стандартизированный код возврата
     */
    int normalizeExitCode(int commandResult) const;

private:
    /**
     * @brief Реестр команд
     */
    std::shared_ptr<CommandRegistry> commandRegistry_;

    /**
     * @brief Фабрика команд
     */
    std::shared_ptr<CommandFactory> commandFactory_;

    /**
     * @brief Флаг инициализации
     */
    bool initialized_;

    /**
     * @brief Маппинг кодов возврата команд для стандартизации
     */
    std::map<int, int> exitCodeMapping_;
};

/**
 * @brief Фабрика для создания CommandBridge
 */
class CommandBridgeFactory {
public:
    /**
     * @brief Создание стандартного Bridge
     */
    static std::shared_ptr<CommandBridge> createDefault();

    /**
     * @brief Создание Bridge с кастомными компонентами
     *
     * @param commandFactory Кастомная фабрика команд
     * @return Настроенный CommandBridge
     */
    static std::shared_ptr<CommandBridge> createWithFactory(
        std::shared_ptr<CommandFactory> commandFactory);
};

} // namespace v8unpack

