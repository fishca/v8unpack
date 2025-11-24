#ifndef COMMANDS_COMMAND_H
#define COMMANDS_COMMAND_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "../messageregistration.h"

namespace v8unpack {

/**
 * @brief Базовый класс для всех команд CLI интерфейса
 *
 * Реализует паттерн Command для обработки команд v8unpack.
 * Каждая команда инкапсулирует логику выполнения определенной операции.
 */
class Command {
protected:
    std::shared_ptr<::MessageRegistrator> logger_;

public:
    /**
     * @brief Конструктор команды
     * @param logger Указатель на регистратор сообщений для логирования
     */
    explicit Command(std::shared_ptr<::MessageRegistrator> logger = nullptr);

    /**
     * @brief Виртуальный деструктор
     */
    virtual ~Command() = default;

    /**
     * @brief Выполнить команду
     * @param args Аргументы командной строки для команды
     * @return Код возврата (0 - успех, отрицательные значения - ошибки)
     */
    virtual int execute(const std::vector<std::string>& args) = 0;

    /**
     * @brief Получить имя команды
     * @return Строковое имя команды
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Получить описание команды
     * @return Описание назначения команды
     */
    virtual std::string getDescription() const = 0;

    /**
     * @brief Показать справку по использованию команды
     */
    virtual void showUsage() const = 0;

    /**
     * @brief Проверить поддержку модификатора
     * @param modifier Имя модификатора (например, "nopack")
     * @return true если модификатор поддерживается
     */
    virtual bool supportsModifier(const std::string& modifier) const;

    /**
     * @brief Установить значение модификатора
     * @param modifier Имя модификатора
     * @param value Значение модификатора
     */
    virtual void setModifier(const std::string& modifier, bool value);
};

} // namespace v8unpack

#endif // COMMANDS_COMMAND_H
