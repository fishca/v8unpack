#ifndef COMMANDS_COMMAND_FACTORY_H
#define COMMANDS_COMMAND_FACTORY_H

#include "CommandRegistry.h"
#include <memory>
#include <memory>

namespace v8unpack {

// Forward declarations for concrete commands
class UnpackCommand;
class PackCommand;
class ParseCommand;
class BuildCommand;
class DeflateCommand;
class DeflateCommand;
class InflateCommand;
class ListCommand;
class VersionCommand;
class HelpCommand;
class ExampleCommand;
class BatCommand;

/**
 * @brief Фабрика для создания и настройки реестра команд
 *
 * Реализует паттерн Factory для централизованного создания команд и их регистрации.
 * Отвечает за инициализацию всех доступных команд с правильными зависимостями.
 */
class CommandFactory {
private:
    std::shared_ptr<::MessageRegistrator> logger_;

public:
    /**
     * @brief Конструктор фабрики команд
     * @param logger Указатель на регистратор сообщений для всех команд
     */
    explicit CommandFactory(std::shared_ptr<::MessageRegistrator> logger = nullptr);

    /**
     * @brief Деструктор
     */
    ~CommandFactory() = default;

    /**
     * @brief Создать и настроить реестр команд
     *
     * Создает реестр команд и регистрирует в нем все доступные команды
     * с правильными зависимостями (logger и т.д.)
     *
     * @return Уникальный указатель на настроенный реестр команд
     */
    std::shared_ptr<CommandRegistry> createRegistry();

private:
    /**
     * @brief Создать команду распаковки
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createUnpackCommand();

    /**
     * @brief Создать команду упаковки
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createPackCommand();

    /**
     * @brief Создать команду парсинга
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createParseCommand();

    /**
     * @brief Создать команду сборки
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createBuildCommand();

    /**
     * @brief Создать команду сжатия (deflate)
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createDeflateCommand();

    /**
     * @brief Создать команду распаковки (inflate)
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createInflateCommand();

    /**
     * @brief Создать команду списка файлов
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createListCommand();

    /**
     * @brief Создать команду версии
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createVersionCommand();

    /**
     * @brief Создать команду справки
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createHelpCommand();

    /**
     * @brief Создать команду примеров использования
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createExampleCommand();

    /**
     * @brief Создать команду генерации bat файла
     * @return Уникальный указатель на команду
     */
    std::unique_ptr<Command> createBatCommand();
};

} // namespace v8unpack

#endif // COMMANDS_COMMAND_FACTORY_H
