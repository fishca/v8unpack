#include "BatchProcessor.h"

#include <fstream>
#include <iostream>
#include <chrono>
#include <algorithm>
#include <sstream>
#include <boost/filesystem.hpp>
#include <set>
#include <iomanip>

namespace fs = boost::filesystem;

namespace v8unpack {

BatchProcessor::BatchProcessor()
    : lastResults_()
    , operationBackups_()
    , verbose_(true)
    , operationTimeoutSeconds_(300.0)
    , maxConsecutiveErrors_(3) {
    // Настройки по умолчанию уже установлены
}

BatchResult BatchProcessor::processBatchFile(const std::string& batchFilePath,
                                            bool stopOnFirstError) {
    // Очищаем предыдущие результаты
    lastResults_.clear();
    operationBackups_.clear();

    // Читаем batch файл
    std::vector<std::string> fileLines;
    ErrorInfo readError = readBatchFile(batchFilePath, fileLines);

    if (!readError.isSuccess()) {
        BatchResult result;
        result.errors.push_back(readError);
        result.errorCount = 1;
        return result;
    }

    // Создаем factory функцию для преобразования строк в операции
    // TODO: Реализовать полноценный factory на основе Application
    auto operationFactory = [this](const std::string& line) -> BatchOperation {
        BatchOperation op;
        op.description = "Execute: " + line;
        op.operationId = std::to_string(std::hash<std::string>()(line));
        op.isCritical = line.find("critical") != std::string::npos;

        // Заглушка для операции - возвращаем success
        op.operation = []() -> ErrorInfo {
            return ErrorInfo(ErrorCode::SUCCESS);
        };

        return op;
    };

    // Разбираем файл и получаем список операций
    std::vector<BatchOperation> operations = parseBatchFile(batchFilePath, operationFactory);

    // Валидируем зависимости
    std::vector<ErrorInfo> dependencyErrors = validateOperationDependencies(operations);
    if (!dependencyErrors.empty()) {
        BatchResult result;
        result.errors = dependencyErrors;
        result.errorCount = dependencyErrors.size();
        return result;
    }

    // Фильтруем дубликаты и сортируем по приоритету
    operations = filterDuplicateOperations(operations);
    operations = sortOperationsByPriority(operations);

    // Выполняем операции
    return processOperations(operations, stopOnFirstError);
}

BatchResult BatchProcessor::processOperations(const std::vector<BatchOperation>& operations,
                                             bool stopOnFirstError) {
    BatchResult result;
    size_t consecutiveErrors = 0;

    // Очищаем предыдущие результаты
    lastResults_.clear();
    operationBackups_.clear();

    size_t totalOperations = operations.size();

    for (size_t i = 0; i < totalOperations; ++i) {
        const BatchOperation& op = operations[i];

        // Выполняем операцию
        BatchOperationResult opResult = executeOperation(op, i + 1);

        // Сохраняем результат для отчетов
        lastResults_.push_back(opResult);

        // Обновляем счетчики
        if (opResult.success) {
            result.successCount++;
            consecutiveErrors = 0; // Сбрасываем счетчик подряд идущих ошибок
        } else {
            result.errorCount++;
            consecutiveErrors++;
            result.errors.push_back(opResult.error);
        }

        // Показываем прогресс выполнения
        double progress = calculateProgress(i + 1, totalOperations);
        logProgress(opResult, progress);

        // Проверяем условия остановки
        if (stopOnFirstError && !opResult.success) {
            break;
        }

        // Критическая ошибка - останавливаемся
        if (!opResult.success && op.isCritical) {
            break;
        }

        // Слишком много подряд ошибок - останавливаемся
        if (consecutiveErrors >= maxConsecutiveErrors_) {
            break;
        }
    }

    return result;
}

std::vector<BatchOperation> BatchProcessor::parseBatchFile(
    const std::string& batchFilePath,
    std::function<BatchOperation(const std::string&)> operationFactory) {

    std::vector<BatchOperation> operations;
    std::vector<std::string> lines;

    // Читаем файл
    ErrorInfo readError = readBatchFile(batchFilePath, lines);
    if (!readError.isSuccess()) {
        return operations;
    }

    // Парсим каждую строку в операцию
    for (const std::string& line : lines) {
        // Пропускаем комментарии и пустые строки
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        try {
            BatchOperation op = operationFactory(line);
            if (!op.operationId.empty()) {
                operations.push_back(op);
            }
        } catch (const std::exception& e) {
            // Логируем ошибку парсинга
            if (verbose_) {
                std::cerr << "Warning: Failed to parse operation line: " << line << " (" << e.what() << ")" << std::endl;
            }
        }
    }

    return operations;
}

std::string BatchProcessor::generateDetailedReport(const std::vector<BatchOperationResult>& results) const {
    std::stringstream ss;

    ss << "=========================================\n";
    ss << "Batch Processing Detailed Report\n";
    ss << "=========================================\n\n";

    if (results.empty()) {
        ss << "No operations executed.\n";
        return ss.str();
    }

    // Статистика
    size_t successCount = 0;
    size_t errorCount = 0;
    double totalTime = 0.0;

    for (const auto& result : results) {
        if (result.success) successCount++;
        else errorCount++;

        totalTime += result.executionTimeSeconds;
    }

    ss << "Summary:\n";
    ss << "- Total Operations: " << results.size() << "\n";
    ss << "- Successful: " << successCount << "\n";
    ss << "- Failed: " << errorCount << "\n";
    ss << "- Success Rate: " << (results.size() > 0 ? (successCount * 100.0 / results.size()) : 0) << "%\n";
    ss << "- Total Time: " << std::fixed << std::setprecision(2) << totalTime << " seconds\n";
    ss << "\n";

    // Детализация по операциям
    ss << "Operation Details:\n";
    ss << "----------------\n";

    for (const auto& result : results) {
        ss << result.operationNumber << ". ";
        ss << (result.success ? "[SUCCESS]" : "[FAILED] ");
        ss << result.operationDescription << "\n";

        if (!result.success) {
            ss << "    Error: " << result.error.message() << "\n";
            if (!result.error.details().empty()) {
                ss << "    Details: " << result.error.details() << "\n";
            }
        }

        ss << "    Time: " << std::fixed << std::setprecision(3) << result.executionTimeSeconds << "s\n";

        if (&result != &results.back()) {
            ss << "\n";
        }
    }

    // Выводим среднее время выполнения
    if (!results.empty()) {
        ss << "\nPerformance:\n";
        ss << "- Average time per operation: " << std::fixed << std::setprecision(3)
           << (totalTime / results.size()) << " seconds\n";

        if (successCount < results.size()) {
            ss << "- Operations that failed: ";
            for (size_t i = 0; i < results.size(); ++i) {
                if (!results[i].success) {
                    ss << (i + 1);
                    if (i < results.size() - 1) ss << ",";
                    ss << " ";
                }
            }
            ss << "\n";
        }
    }

    ss << "\n=========================================\n";
    return ss.str();
}

const std::vector<BatchOperationResult>& BatchProcessor::getLastResults() const {
    return lastResults_;
}

bool BatchProcessor::canRollback(const std::vector<BatchOperationResult>& results) const {
    // Проверяем что все успешные операции имели backup
    for (const auto& result : results) {
        if (result.success) {
            // Ищем backup для этой операции
            auto it = operationBackups_.find(std::to_string(result.operationNumber));
            if (it == operationBackups_.end()) {
                return false; // Нет backup для успешной операции
            }
        }
    }

    return true;
}

BatchResult BatchProcessor::rollbackOperations(const std::vector<BatchOperationResult>& results) {
    BatchResult rollbackResult;

    if (!canRollback(results)) {
        rollbackResult.errorCount = 1;
        rollbackResult.errors.push_back(
            ErrorInfo(ErrorCode::OPERATION_FAILED, "Cannot rollback: some operations have no backup")
        );
        return rollbackResult;
    }

    // Откатываем в обратном порядке
    for (auto it = results.rbegin(); it != results.rend(); ++it) {
        if (it->success) {
            // Откатываем успешную операцию
            auto backupIt = operationBackups_.find(std::to_string(it->operationNumber));

            if (backupIt != operationBackups_.end()) {
                ErrorInfo restoreResult = restoreFromBackup(backupIt->second);

                if (restoreResult.isSuccess()) {
                    rollbackResult.successCount++;
                } else {
                    rollbackResult.errorCount++;
                    rollbackResult.errors.push_back(restoreResult);
                }
            }
        }
    }

    return rollbackResult;
}

BatchOperationResult BatchProcessor::executeOperation(const BatchOperation& operation,
                                                     size_t operationNumber) {
    BatchOperationResult result;
    result.operationNumber = operationNumber;
    result.operationDescription = operation.description;

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Создаем backup если возможно
        if (!operation.isCritical) {
            std::string backupId = createBackup(operation);
            if (!backupId.empty()) {
                operationBackups_[operation.operationId] = backupId;
            }
        }

        // Выполняем операцию
        ErrorInfo error = operation.operation();

        // Вычисляем время выполнения
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        result.executionTimeSeconds = static_cast<double>(duration.count()) / 1000.0;

        if (error.isSuccess()) {
            result.success = true;
        } else {
            result.success = false;
            result.error = error;
        }

    } catch (const std::exception& e) {
        // Обработка исключений
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
        result.executionTimeSeconds = static_cast<double>(duration.count()) / 1000.0;

        result.success = false;
        result.error = ErrorInfo(ErrorCode::UNKNOWN_ERROR,
                                std::string("Exception during execution: ") + e.what());
    }

    return result;
}

ErrorInfo BatchProcessor::readBatchFile(const std::string& filePath,
                                       std::vector<std::string>& lines) const {
    lines.clear();

    try {
        if (!fs::exists(filePath)) {
            return ErrorInfo(ErrorCode::FILE_NOT_FOUND, "Batch file not found", "", filePath);
        }

        if (!fs::is_regular_file(filePath)) {
            return ErrorInfo(ErrorCode::INVALID_FILE_PATH, "Path is not a regular file", "", filePath);
        }

        std::ifstream file(filePath);
        if (!file.is_open()) {
            return ErrorInfo(ErrorCode::FILE_READ_ERROR, "Cannot open batch file", "", filePath);
        }

        std::string line;
        while (std::getline(file, line)) {
            // Удаляем пробелы и табы с конца
            line.erase(line.find_last_not_of(" \t") + 1);
            lines.push_back(line);
        }

        file.close();
        return ErrorInfo(ErrorCode::SUCCESS);

    } catch (const std::exception& e) {
        return ErrorInfo(ErrorCode::FILE_READ_ERROR,
                        std::string("Error reading batch file: ") + e.what(), "", filePath);
    }
}

std::vector<BatchOperation> BatchProcessor::filterDuplicateOperations(
    const std::vector<BatchOperation>& operations) const {

    std::vector<BatchOperation> filtered;
    std::set<std::string> seenIds;

    for (const auto& op : operations) {
        if (seenIds.find(op.operationId) == seenIds.end()) {
            seenIds.insert(op.operationId);
            filtered.push_back(op);
        } else if (verbose_) {
            std::cout << "Warning: Duplicate operation found: " << op.description << std::endl;
        }
    }

    return filtered;
}

std::vector<BatchOperation> BatchProcessor::sortOperationsByPriority(
    const std::vector<BatchOperation>& operations) const {

    // TODO: Реализовать приоритезацию операций
    // Пока возвращаем как есть
    return operations;
}

std::vector<ErrorInfo> BatchProcessor::validateOperationDependencies(
    const std::vector<BatchOperation>& operations) const {

    // TODO: Реализовать проверку зависимостей
    // Пока возвращаем пустой список (нет ошибок)
    return {};
}

double BatchProcessor::calculateProgress(size_t currentOperation, size_t totalOperations) const {
    if (totalOperations == 0) return 100.0;
    return (static_cast<double>(currentOperation) / totalOperations) * 100.0;
}

void BatchProcessor::logProgress(const BatchOperationResult& operationResult, double progress) const {
    if (verbose_) {
        std::cout << "[" << std::fixed << std::setprecision(1) << progress << "%] ";
        std::cout << "Operation " << operationResult.operationNumber << ": ";

        if (operationResult.success) {
            std::cout << "✓ " << operationResult.operationDescription;
        } else {
            std::cout << "✗ " << operationResult.operationDescription;
            std::cout << " (Error: " << operationResult.error.message() << ")";
        }

        std::cout << " [" << std::fixed << std::setprecision(2) << operationResult.executionTimeSeconds << "s]";
        std::cout << std::endl;
    }
}

std::string BatchProcessor::createBackup(const BatchOperation& operation) {
    // TODO: Реализовать создание бэкапа для операций
    // Пока возвращаем фиктивный ID
    return "backup_" + operation.operationId;
}

ErrorInfo BatchProcessor::restoreFromBackup(const std::string& backupId) {
    // TODO: Реализовать восстановление из бэкапа
    // Пока возвращаем успех
    return ErrorInfo(ErrorCode::SUCCESS);
}

// =========== Реализация BatchUtils ===========

bool BatchUtils::isBatchFile(const std::string& filePath) {
    std::string lowerPath = filePath;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    return lowerPath.find(".batch") != std::string::npos ||
           lowerPath.find("list") != std::string::npos ||
           lowerPath == "batch" || lowerPath == "list";
}

ErrorInfo BatchUtils::createEmptyBatchFile(const std::string& filePath) {
    try {
        std::ofstream file(filePath);
        if (!file.is_open()) {
            return ErrorInfo(ErrorCode::FILE_WRITE_ERROR, "Cannot create batch file", "", filePath);
        }

        file << "# V8Unpack Batch File\n";
        file << "# Each line represents one operation to execute\n";
        file << "# Lines starting with # are comments\n";
        file << "# Example: unpack inputfile.cf output/\n";
        file << "\n";

        file.close();
        return ErrorInfo(ErrorCode::SUCCESS);

    } catch (const std::exception& e) {
        return ErrorInfo(ErrorCode::FILE_WRITE_ERROR,
                        std::string("Error creating batch file: ") + e.what(), "", filePath);
    }
}

ErrorInfo BatchUtils::appendOperationToBatch(const std::string& filePath,
                                            const std::string& operationLine) {
    try {
        std::ofstream file(filePath, std::ios::app);
        if (!file.is_open()) {
            return ErrorInfo(ErrorCode::FILE_WRITE_ERROR, "Cannot append to batch file", "", filePath);
        }

        file << operationLine << "\n";
        file.close();

        return ErrorInfo(ErrorCode::SUCCESS);

    } catch (const std::exception& e) {
        return ErrorInfo(ErrorCode::FILE_WRITE_ERROR,
                        std::string("Error appending to batch file: ") + e.what(), "", filePath);
    }
}

std::vector<ErrorInfo> BatchUtils::validateBatchFile(const std::string& filePath) {
    std::vector<ErrorInfo> errors;

    if (!fs::exists(filePath)) {
        errors.push_back(ErrorInfo(ErrorCode::FILE_NOT_FOUND, "Batch file not found", "", filePath));
        return errors;
    }

    std::vector<std::string> lines;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        errors.push_back(ErrorInfo(ErrorCode::FILE_READ_ERROR, "Cannot open batch file", "", filePath));
        return errors;
    }

    std::string line;
    size_t lineNumber = 0;

    while (std::getline(file, line)) {
        lineNumber++;

        // Удаляем пробелы с начала и конца
        line.erase(0, line.find_first_not_of(" \t"));
        line.erase(line.find_last_not_of(" \t") + 1);

        // Пропускаем пустые строки и комментарии
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }

        // Базовая валидация формата операции
        if (line.length() < 3) { // Минимум команда + пробел + аргумент
            errors.push_back(ErrorInfo(ErrorCode::VALIDATION_ERROR,
                                      "Operation too short", line,
                                      filePath + ":" + std::to_string(lineNumber)));
        }

        // TODO: Добавить более глубокую валидацию операций V8Unpack
    }

    file.close();
    return errors;
}

std::string BatchUtils::formatBatchResult(const BatchResult& result) {
    std::stringstream ss;

    ss << "Batch Execution Result:\n";
    ss << "- Total: " << result.totalOperations() << "\n";
    ss << "- Success: " << result.successCount << "\n";
    ss << "- Failed: " << result.errorCount << "\n";
    ss << "- Success Rate: " << std::fixed << std::setprecision(1) << result.successRate() << "%\n";
    ss << "- Overall: " << (result.overallSuccess() ? "SUCCESS" : "FAILED") << "\n";

    if (!result.errors.empty()) {
        ss << "\nErrors:\n";
        for (size_t i = 0; i < result.errors.size(); ++i) {
            ss << (i + 1) << ". " << result.errors[i].message() << "\n";
        }
    }

    return ss.str();
}

ErrorInfo BatchUtils::exportResultsToFile(const std::vector<BatchOperationResult>& results,
                                         const std::string& outputFilePath) {
    try {
        std::ofstream file(outputFilePath);
        if (!file.is_open()) {
            return ErrorInfo(ErrorCode::FILE_WRITE_ERROR, "Cannot create results file", "", outputFilePath);
        }

        // Заголовок
        file << "# V8Unpack Batch Execution Results\n";
        file << "# Generated: " << std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()) << "\n";
        file << "\n";

        // Статистика
        size_t successCount = 0, errorCount = 0;
        double totalTime = 0.0;

        for (const auto& result : results) {
            if (result.success) successCount++;
            else errorCount++;
            totalTime += result.executionTimeSeconds;
        }

        file << "Summary:\n";
        file << "- Operations: " << results.size() << "\n";
        file << "- Success: " << successCount << "\n";
        file << "- Failed: " << errorCount << "\n";
        file << "- Total Time: " << std::fixed << std::setprecision(2) << totalTime << "s\n";
        file << "\n";

        // Детализация
        file << "Operations:\n";
        for (const auto& result : results) {
            file << result.operationNumber << ",";
            file << (result.success ? "SUCCESS" : "FAILED") << ",";
            file << "\"" << result.operationDescription << "\",";
            file << std::fixed << std::setprecision(3) << result.executionTimeSeconds;
            file << "\n";
        }

        file.close();
        return ErrorInfo(ErrorCode::SUCCESS);

    } catch (const std::exception& e) {
        return ErrorInfo(ErrorCode::FILE_WRITE_ERROR,
                        std::string("Error exporting results: ") + e.what(), "", outputFilePath);
    }
}

} // namespace v8unpack
