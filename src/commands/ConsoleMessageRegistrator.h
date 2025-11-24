#ifndef COMMANDS_CONSOLE_MESSAGE_REGISTRATOR_H
#define COMMANDS_CONSOLE_MESSAGE_REGISTRATOR_H

#include "../messageregistration.h"
#include <iostream>

namespace v8unpack {

/**
 * @brief Простая реализация MessageRegistrator для вывода в консоль
 *
 * Реализует интерфейс MessageRegistrator для вывода сообщений
 * в стандартный вывод и стандартный поток ошибок.
 */
class ConsoleMessageRegistrator : public ::MessageRegistrator {
public:
    /**
     * @brief Конструктор
     */
    ConsoleMessageRegistrator();

    /**
     * @brief Деструктор
     */
    ~ConsoleMessageRegistrator() override;

    /**
     * @brief Добавить сообщение
     * @param description Описание сообщения
     * @param mstate Состояние сообщения
     * @param param Дополнительные параметры (не используется)
     */
    void AddMessage(const String& description, const MessageState mstate,
                   const TStringList* param = nullptr) override;

    /**
     * @brief Вывести статус
     * @param message Сообщение статуса
     */
    void Status(const String& message) override;
};

} // namespace v8unpack

#endif // COMMANDS_CONSOLE_MESSAGE_REGISTRATOR_H
