#ifndef UTILS_STRING_CONVERTERS_H
#define UTILS_STRING_CONVERTERS_H

#include <string>
#include <locale>

namespace v8unpack {

/**
 * @brief Утилиты для конвертации строк и работы с кодировками
 *
 * Содержит функции для конвертации между различными типами строк,
 * удаления BOM маркеров, работы с локалями и кодировками.
 *
 * Перенесено из main.cpp для улучшения организации кода и повторного использования.
 */
class StringConverters {
public:
    /**
     * @brief Конвертация std::wstring в std::string с использованием UTF-8
     *
     * Использует wstring_convert с codecvt_utf8 для корректной конвертации Unicode.
     *
     * @param wstr Исходная wide string
     * @return std::string в UTF-8 кодировке
     */
    static std::string wstringToString(const std::wstring& wstr);

    /**
     * @brief Альтернативная конвертация std::wstring в std::string
     *
     * Использует платформозависимые функции wcstombs для конвертации.
     * Более низкоуровневый подход с ручным управлением буфером.
     *
     * @param wstr Исходная wide string
     * @return std::string или пустая строка при ошибке
     */
    static std::string WStringToString(const std::wstring& wstr);

    /**
     * @brief Удаление BOM маркера UTF-8 из строки
     *
     * Если строка начинается с последовательности 0xEF 0xBB 0xBF,
     * эта последовательность удаляется.
     *
     * @param inputStr Исходная строка
     * @return Строка без BOM маркера
     */
    static std::string removeBOM(const std::string& inputStr);

    /**
     * @brief Удаление BOM маркера UTF-16 из wide строки
     *
     * Удаляет ведущий символ BOM (0xFE или 0xFF) из wide строки.
     *
     * @param inputStr Исходная wide строка
     * @return Wide строка без BOM маркера
     */
    static std::wstring removeBOMutf16(const std::wstring& inputStr);

    /**
     * @brief Конвертация UTF-8 строки с учетом локали
     *
     * Конвертирует UTF-8 строку в locale-специфичное представление
     * с использованием narrow facet'а.
     *
     * @param utf8str Исходная UTF-8 строка
     * @param loc Локаль для конвертации
     * @return Конвертированная строка
     */
    static std::string utf8_to_string(const char* utf8str, const std::locale& loc);

    /**
     * @brief Конвертация из UTF-8 в locale-специфичную строку
     *
     * Использует wstring_convert для промежуточной конвертации через wide строку,
     * затем применяет narrow facet для финальной конвертации.
     *
     * @param str Исходная UTF-8 строка
     * @param loc Локаль для конвертации (по умолчанию - классическая)
     * @return Конвертированная строка или строка с placeholder символами
     */
    static std::string from_utf8(const std::string& str, const std::locale& loc = std::locale{});
};

} // namespace v8unpack

#endif // UTILS_STRING_CONVERTERS_H
