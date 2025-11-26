#ifndef COMMANDS_COMMAND_REGISTRY_H
#define COMMANDS_COMMAND_REGISTRY_H

#include "Command.h"
#include <map>
#include <vector>
#include <memory>
#include <iostream>
#include <algorithm>

// Forward declaration for MessageRegistrator from global namespace
class MessageRegistrator;

namespace v8unpack {

/**
 * @brief Реестр команд для управления доступными командами CLI
 *
 * Реализует паттерн Registry для централизованного хранения и доступа к командам.
 * Поддерживает регистрацию команд, поиск по имени и отображение справки.
 */
class CommandRegistry {
private:
    std::map<std::string, std::unique_ptr<Command>> commands_;
    std::shared_ptr<::MessageRegistrator> logger_;

    /**
     * @brief Внутренний метод получения команды без обработки алиасов
     * @param name Имя команды
     * @return Указатель на команду или nullptr
     */
    Command* getCommandInternal(const std::string& name) const;

public:
    /**
     * @brief Конструктор реестра команд
     * @param logger Указатель на регистратор сообщений
     */
    explicit CommandRegistry(std::shared_ptr<::MessageRegistrator> logger = nullptr);

    /**
     * @brief Деструктор
     */
    ~CommandRegistry() = default;

    /**
     * @brief Зарегистрировать новую команду
     * @param command Уникальный указатель на команду
     */
    void registerCommand(std::unique_ptr<Command> command);

    /**
     * @brief Получить команду по имени
     *
     * Поддерживает алиасы и префиксы команд (-unpack -> unpack, u -> unpack)
     * @param name Имя команды (с или без префикса -)
     * @return Указатель на команду или nullptr если команда не найдена
     */
    Command* getCommand(const std::string& name) const;

    /**
     * @brief Получить список доступных команд
     * @return Вектор имен команд
     */
    std::vector<std::string> getAvailableCommands() const;

    /**
     * @brief Проверить существование команды
     * @param name Имя команды
     * @return true если команда существует
     */
    bool hasCommand(const std::string& name) const;

    /**
     * @brief Выполнить команду с указанными аргументами
     *
     * @param commandName Имя команды
     * @param args Аргументы команды
     * @return Код возврата (0 - успех)
     */
    int executeCommand(const std::string& commandName, const std::vector<std::string>& args);

    /**
     * @brief Показать общую справку по всем командам
     */
    void showHelp() const;

    /**
     * @brief Показать справку по конкретной команде
     * @param commandName Имя команды
     */
    void showCommandHelp(const std::string& commandName) const;

    /**
     * @brief Получить количество зарегистрированных команд
     * @return Количество команд
     */
    size_t getCommandCount() const;
};

} // namespace v8unpack

#endif // COMMANDS_COMMAND_REGISTRY_H
