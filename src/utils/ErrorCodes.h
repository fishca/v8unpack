#ifndef UTILS_ERROR_CODES_H
#define UTILS_ERROR_CODES_H

#include <string>
#include <map>

namespace v8unpack {

/**
 * @brief Коды ошибок приложения V8Unpack
 *
 * Централизованный способ обработки ошибок с понятными кодами
 * и описаниями. Заменяет использование магических чисел в коде.
 */
enum class ErrorCode {
    // Успешные операции = 0
    SUCCESS = 0,

    // Ошибки аргументов и входных данных = 1-99
    INVALID_ARGUMENTS = 1,
    MISSING_REQUIRED_ARG = 2,
    INVALID_COMMAND = 3,
    FILE_NOT_FOUND = 4,
    DIRECTORY_NOT_FOUND = 5,
    INVALID_FILE_PATH = 6,
    VALIDATION_ERROR = 7,

    // Ошибки файловых операций = 100-199
    FILE_READ_ERROR = 100,
    FILE_WRITE_ERROR = 101,
    FILE_ACCESS_DENIED = 102,
    INSUFFICIENT_DISK_SPACE = 103,
    FILE_CORRUPTED = 104,

    // Ошибки V8 формата = 200-299
    INVALID_V8_FORMAT = 200,
    MISSING_V8_SIGNATURE = 201,
    V8_VERSION_MISMATCH = 202,
    METADATA_CORRUPTED = 203,

    // Ошибки сжатия/распаковки = 300-399
    COMPRESSION_FAILED = 300,
    DECOMPRESSION_FAILED = 301,
    UNSUPPORTED_COMPRESSION = 302,

    // Ошибки метаданных = 400-499
    METADATA_PARSING_ERROR = 400,
    INVALID_ROOT_STRUCTURE = 401,
    MISSING_CONFIGURATION_GUID = 402,
    METADATA_SERIALIZATION_ERROR = 403,

    // Ошибки команд = 500-599
    COMMAND_EXECUTION_FAILED = 500,
    COMMAND_TIMEOUT = 501,
    COMMAND_CANCELLED = 502,
    OPERATION_FAILED = 503,

    // Системные ошибки = 600-699
    OUT_OF_MEMORY = 600,
    SYSTEM_CALL_FAILED = 601,

    // Резерв для будущих ошибок = 700-999
    UNKNOWN_ERROR = 999
};

/**
 * @brief Информация об ошибке с деталями
 *
 * Содержит код ошибки, текстовое описание и дополнительные детали
 * для диагностики и обработки ошибок.
 */
class ErrorInfo {
public:
    /**
     * @brief Конструктор по умолчанию
     *
     * Создает ErrorInfo с кодом SUCCESS
     */
    ErrorInfo();

    /**
     * @brief Конструктор с кодом ошибки
     *
     * @param code Код ошибки
     */
    explicit ErrorInfo(ErrorCode code);

    /**
     * @brief Конструктор с кодом и сообщением
     *
     * @param code Код ошибки
     * @param message Описание ошибки
     */
    ErrorInfo(ErrorCode code, const std::string& message);

    /**
     * @brief Конструктор с полными деталями
     *
     * @param code Код ошибки
     * @param message Описание ошибки
     * @param details Дополнительные детали
     * @param context Контекст (файл, операция и т.д.)
     */
    ErrorInfo(ErrorCode code, const std::string& message,
              const std::string& details, const std::string& context = "");

    /**
     * @brief Проверяет, является ли ошибка успешной операцией
     *
     * @return true если код == SUCCESS, false иначе
     */
    bool isSuccess() const;

    /**
     * @brief Проверяет, является ли ошибка связанной с файлами
     *
     * @return true если ошибка относится к файловым операциям
     */
    bool isFileError() const;

    /**
     * @brief Проверяет, является ли ошибка связанной с метаданными
     *
     * @return true если ошибка относится к метаданным V8
     */
    bool isMetadataError() const;

    /**
     * @brief Возвращает код ошибки
     */
    ErrorCode code() const;

    /**
     * @brief Возвращает текстовое описание ошибки
     */
    const std::string& message() const;

    /**
     * @brief Возвращает дополнительные детали
     */
    const std::string& details() const;

    /**
     * @brief Возвращает контекст ошибки
     */
    const std::string& context() const;

    /**
     * @brief Преобразует код ошибки в числовое значение
     *
     * @param code Код ошибки
     * @return Целочисленное представление кода
     */
    static int toInt(ErrorCode code);

    /**
     * @brief Преобразует числовое значение в код ошибки
     *
     * @param value Целочисленное значение
     * @return Код ошибки или UNKNOWN_ERROR если не распознано
     */
    static ErrorCode fromInt(int value);

    /**
     * @brief Возвращает текстовое описание кода ошибки
     *
     * @param code Код ошибки
     * @return Строка с описанием
     */
    static std::string getDefaultMessage(ErrorCode code);

    /**
     * @brief Оператор преобразования к bool (true если успех)
     */
    explicit operator bool() const;

private:
    ErrorCode m_code;
    std::string m_message;
    std::string m_details;
    std::string m_context;
};

/**
 * @brief Утилитарные функции для работы с ошибками
 */
class ErrorUtils {
public:
    /**
     * @brief Создает ErrorInfo для ошибок файлов
     *
     * @param filename Имя файла
     * @param operation Описание операции
     * @param additionalInfo Дополнительная информация
     * @return ErrorInfo с кодом FILE_* ошибки
     */
    static ErrorInfo createFileError(const std::string& filename,
                                    const std::string& operation,
                                    const std::string& additionalInfo = "");

    /**
     * @brief Создает ErrorInfo для ошибок аргументов
     *
     * @param argument Аргумент вызвавший ошибку
     * @param expected Ожидаемое значение
     * @param actual Фактическое значение
     * @return ErrorInfo с кодом INVALID_ARGUMENTS
     */
    static ErrorInfo createArgumentError(const std::string& argument,
                                       const std::string& expected,
                                       const std::string& actual);

    /**
     * @brief Создает ErrorInfo для ошибок V8 формата
     *
     * @param issue Описание проблемы
     * @param filename Имя файла
     * @return ErrorInfo с кодом INVALID_V8_FORMAT
     */
    static ErrorInfo createV8FormatError(const std::string& issue,
                                       const std::string& filename = "");
};

/**
 * @brief Глобальные константы кодов ошибок для backward compatibility
 *
 * Эти константы будут постепенно заменены на ErrorCode enum
 * в процессе рефакторинга остальных частей кода.
 */
namespace LegacyErrorCodes {
    const int V8UNPACK_SHOW_USAGE = -1;
    const int SUCCESS = 0;
    // Другие legacy коды могут быть добавлены по мере анализа кода
}

} // namespace v8unpack

#endif // UTILS_ERROR_CODES_H
