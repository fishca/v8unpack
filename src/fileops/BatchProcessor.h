#ifndef FILEOPS_BATCH_PROCESSOR_H
#define FILEOPS_BATCH_PROCESSOR_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>

#include "../utils/ErrorCodes.h"

namespace v8unpack {

/**
 * @brief Результат выполнения пакетной операции
 */
struct BatchResult {
    /**
     * @brief Количество успешно выполненных операций
     */
    size_t successCount = 0;

    /**
     * @brief Количество неудачных операций
     */
    size_t errorCount = 0;

    /**
     * @brief Детальные ошибки по каждой операции
     */
    std::vector<ErrorInfo> errors;

    /**
     * @brief Успешность всего пакета (success если не было критичных ошибок)
     */
    bool overallSuccess() const {
        return errorCount == 0 || successCount > errorCount;
    }

    /**
     * @brief Полное количество операций
     */
    size_t totalOperations() const {
        return successCount + errorCount;
    }

    /**
     * @brief Процент успешных операций
     */
    double successRate() const {
        size_t total = totalOperations();
        return total > 0 ? (static_cast<double>(successCount) / total) * 100.0 : 0.0;
    }
};

/**
 * @brief Результирующая запись для одной операции в пакете
 */
struct BatchOperationResult {
    /**
     * @brief Номер операции в пакете (начиная с 1)
     */
    size_t operationNumber;

    /**
     * @brief Описание выполненной операции
     */
    std::string operationDescription;

    /**
     * @brief Успешность выполнения
     */
    bool success;

    /**
     * @brief Информация об ошибке (если success == false)
     */
    ErrorInfo error;

    /**
     * @brief Время выполнения операции (в секундах)
     */
    double executionTimeSeconds;
};

/**
 * @brief Операция в пакете команд
 */
struct BatchOperation {
    /**
     * @brief Описание команды для логов
     */
    std::string description;

    /**
     * @brief Функция выполнения операции
     */
    std::function<ErrorInfo()> operation;

    /**
     * @brief Флаг, является ли операция критичной (остановить пакет при ошибке)
     */
    bool isCritical = false;

    /**
     * @brief Уникальный ID операции для отслеживания
     */
    std::string operationId;
};

/**
 * @brief Процессор пакетного выполнения команд V8Unpack
 *
 * Отвечает за:
 * - Чтение списка операций из файла
 * - Последовательное выполнение операций
 * - Отчетность о результатах
 * - Обработку ошибок и критических сбоев
 */
class BatchProcessor {
public:
    /**
     * @brief Конструктор
     */
    BatchProcessor();

    /**
     * @brief Деструктор
     */
    ~BatchProcessor() = default;

    /**
     * @brief Выполнение пакета операций из файла
     *
     * @param batchFilePath Путь к файлу с описанием операций
     * @param stopOnFirstError Остановить ли выполнение при первой ошибке
     * @return Результат выполнения пакета
     */
    BatchResult processBatchFile(const std::string& batchFilePath,
                                bool stopOnFirstError = false);

    /**
     * @brief Выполнение списка операций
     *
     * @param operations Список операций для выполнения
     * @param stopOnFirstError Остановить ли выполнение при первой ошибке
     * @return Результат выполнения пакета
     */
    BatchResult processOperations(const std::vector<BatchOperation>& operations,
                                 bool stopOnFirstError = false);

    /**
     * @brief Разбор batch файла и создание списка операций
     *
     * @param batchFilePath Путь к batch файлу
     * @param operationFactory Функция создания операций из строки
     * @return Список разобранных операций
     */
    std::vector<BatchOperation> parseBatchFile(
        const std::string& batchFilePath,
        std::function<BatchOperation(const std::string&)> operationFactory);

    /**
     * @brief Получение подробного отчета о выполнении
     *
     * @param results Результаты выполнения всех операций
     * @return Текстовый отчет
     */
    std::string generateDetailedReport(const std::vector<BatchOperationResult>& results) const;

    /**
     * @brief Получение результатов последнего выполнения
     */
    const std::vector<BatchOperationResult>& getLastResults() const;

    /**
     * @brief Проверка возможности отката пакета
     *
     * @param results Результаты выполнения операций
     * @return Можно ли откатить пакет
     */
    bool canRollback(const std::vector<BatchOperationResult>& results) const;

    /**
     * @brief Выполнение отката пакета операций
     *
     * @param results Результаты выполнения для отката
     * @return Результат отката
     */
    BatchResult rollbackOperations(const std::vector<BatchOperationResult>& results);

private:
    /**
     * @brief Выполнение одиночной операции
     *
     * @param operation Операция для выполнения
     * @param operationNumber Номер операции в пакете
     * @return Результат выполнения
     */
    BatchOperationResult executeOperation(const BatchOperation& operation,
                                         size_t operationNumber);

    /**
     * @brief Чтение и разбор содержимого batch файла
     *
     * @param filePath Путь к файлу
     * @return Содержимое файла как строки (по одной operation на строку)
     */
    ErrorInfo readBatchFile(const std::string& filePath,
                           std::vector<std::string>& lines) const;

    /**
     * @brief Фильтрация дубликатов операций
     *
     * @param operations Входной список операций
     * @return Отфильтрованный список без дубликатов
     */
    std::vector<BatchOperation> filterDuplicateOperations(
        const std::vector<BatchOperation>& operations) const;

    /**
     * @brief Сортировка операций по приоритету
     *
     * @param operations Входной список операций
     * @return Отсортированный список
     */
    std::vector<BatchOperation> sortOperationsByPriority(
        const std::vector<BatchOperation>& operations) const;

    /**
     * @brief Проверка зависимостей между операциями
     *
     * @param operations Список операций
     * @return Список ошибок зависимостей
     */
    std::vector<ErrorInfo> validateOperationDependencies(
        const std::vector<BatchOperation>& operations) const;

    /**
     * @brief Вычисление прогресса выполнения
     *
     * @param currentOperation Текущий номер операции
     * @param totalOperations Общее количество операций
     * @return Процент выполнения
     */
    double calculateProgress(size_t currentOperation, size_t totalOperations) const;

    /**
     * @brief Логирование прогресса выполнения
     *
     * @param operationResult Результат выполненной операции
     * @param progress Процент выполнения пакета
     */
    void logProgress(const BatchOperationResult& operationResult, double progress) const;

    /**
     * @brief Резервное копирование перед выполнением операции
     *
     * @param operation Операция для бэкапа
     * @return ID бэкапа или пустая строка при ошибке
     */
    std::string createBackup(const BatchOperation& operation);

    /**
     * @brief Восстановление из бэкапа
     *
     * @param backupId ID бэкапа
     * @return Результат восстановления
     */
    ErrorInfo restoreFromBackup(const std::string& backupId);

private:
    /**
     * @brief Результаты последнего выполнения
     */
    std::vector<BatchOperationResult> lastResults_;

    /**
     * @brief Карта ID операций -> backup ID для rollback
     */
    std::map<std::string, std::string> operationBackups_;

    /**
     * @brief Флаг verbose режима
     */
    bool verbose_ = true;

    /**
     * @brief Максимальное время выполнения операции (секунды)
     */
    double operationTimeoutSeconds_ = 300.0; // 5 минут

    /**
     * @brief Максимальное количество ошибок подряд перед остановкой
     */
    size_t maxConsecutiveErrors_ = 3;
};

/**
 * @brief Утилиты для работы с batch файлами
 */
class BatchUtils {
public:
    /**
     * @brief Проверка является ли файл batch файлом
     */
    static bool isBatchFile(const std::string& filePath);

    /**
     * @brief Создание пустого batch файла с заголовком
     */
    static ErrorInfo createEmptyBatchFile(const std::string& filePath);

    /**
     * @brief Добавление операции в batch файл
     */
    static ErrorInfo appendOperationToBatch(const std::string& filePath,
                                           const std::string& operationLine);

    /**
     * @brief Валидация синтаксиса batch файла
     */
    static std::vector<ErrorInfo> validateBatchFile(const std::string& filePath);

    /**
     * @brief Форматирование BatchResult в читаемую строку
     */
    static std::string formatBatchResult(const BatchResult& result);

    /**
     * @brief Экспорт результатов выполнения в файл
     */
    static ErrorInfo exportResultsToFile(const std::vector<BatchOperationResult>& results,
                                        const std::string& outputFilePath);
};

} // namespace v8unpack

#endif // FILEOPS_BATCH_PROCESSOR_H
