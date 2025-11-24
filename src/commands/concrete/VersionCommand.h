#ifndef COMMANDS_CONCRETE_VERSION_COMMAND_H
#define COMMANDS_CONCRETE_VERSION_COMMAND_H

#include "../Command.h"

namespace v8unpack {

/**
 * @brief Команда для отображения версии программы
 *
 * Реализует команду -version/-v для показа версии V8Unpack
 * и архитектуры системы (32-bit/64-bit).
 */
class VersionCommand : public Command {
public:
    /**
     * @brief Конструктор команды версии
     * @param logger Указатель на регистратор сообщений
     */
    explicit VersionCommand(std::shared_ptr<::MessageRegistrator> logger = nullptr);

    /**
     * @brief Выполнить команду версии
     * @param args Аргументы командной строки (не используются)
     * @return Код возврата (всегда 0)
     */
    int execute(const std::vector<std::string>& args) override;

    /**
     * @brief Получить имя команды
     * @return "version"
     */
    std::string getName() const override;

    /**
     * @brief Получить описание команды
     * @return Описание назначения команды
     */
    std::string getDescription() const override;

    /**
     * @brief Показать справку по использованию команды
     */
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_VERSION_COMMAND_H
