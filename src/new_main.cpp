/**
 * @file new_main.cpp
 * @brief Новый main() с использованием архитектуры рефакторинга
 *
 * Этот файл показывает, как будет выглядеть финальная версия main.cpp
 * после полной интеграции новой архитектуры V8Unpack.
 *
 * Текущий статус: Infrastructure Ready - все компоненты созданы и протестированы
 * Следующий шаг: Integration Phase - поэтапная замена функциональности
 */

#include <iostream>
#include <memory>
#include <chrono>

#include "app/Application.h"
#include "app/ApplicationFactory.h"
#include "metadata/MetadataAnalyzer.h"
#include "fileops/BatchProcessor.h"
#include "utils/ErrorCodes.h"

using namespace v8unpack;

/**
 * @brief Демо функция для тестирования инфраструктуры
 */
void demoInfrastructure() {
    std::cout << "=== V8Unpack 2.0 Infrastructure Demo ===\n\n";

    // 1. Тест Application Factory
    std::cout << "1. Testing Application Factory...\n";
    try {
        auto app = ApplicationFactory::createDefault();
        if (app && app->isInitialized()) {
            std::cout << "   ✓ Application created successfully\n";
            std::cout << "   ✓ Version: " << Application::getVersion() << "\n";
        } else {
            std::cout << "   ✗ Application creation failed\n";
        }
    } catch (const std::exception& e) {
        std::cout << "   ✗ Exception: " << e.what() << "\n";
    }

    // 2. Тест Metadata Analyzer
    std::cout << "\n2. Testing Metadata Analyzer...\n";
    try {
        MetadataAnalyzer analyzer;
        // Создаем тестовый файл для демонстрации
        const std::string testMetadata = "{ \"guid\": \"550e8400-e29b-41d4-a716-446655440000\" }";

        auto result = analyzer.analyzeContent(testMetadata, "demo");
        std::cout << "   ✓ Analysis result: " << (result.valid ? "VALID" : "INVALID") << "\n";
        if (!result.foundTypeGuids.empty()) {
            std::cout << "   ✓ Found " << result.foundTypeGuids.size() << " GUID(s)\n";
        }
    } catch (const std::exception& e) {
        std::cout << "   ✗ Exception: " << e.what() << "\n";
    }

    // 3. Тест Batch Processor
    std::cout << "\n3. Testing Batch Processor...\n";
    try {
        BatchProcessor batchProcessor;

        // Создаем простую тестовую операцию
        BatchOperation testOp;
        testOp.description = "Demo batch operation";
        testOp.operationId = "demo_op_1";
        testOp.isCritical = false;
        testOp.operation = []() -> ErrorInfo {
            return ErrorInfo(ErrorCode::SUCCESS);
        };

        std::vector<BatchOperation> operations = {testOp};
        auto result = batchProcessor.processOperations(operations, false);

        std::cout << "   ✓ Batch result: " << result.successCount << " success, "
                  << result.errorCount << " errors\n";
    } catch (const std::exception& e) {
        std::cout << "   ✗ Exception: " << e.what() << "\n";
    }

    // 4. Тест Error System
    std::cout << "\n4. Testing Error System...\n";
    try {
        auto error = ErrorInfo(ErrorCode::SUCCESS, "Test message");
        std::cout << "   ✓ Error creation: " << (error.isSuccess() ? "SUCCESS" : "FAILED") << "\n";

        auto fileError = ErrorInfo(ErrorCode::FILE_NOT_FOUND, "File not found", "test.txt");
        std::cout << "   ✓ File error: " << fileError.message() << "\n";
    } catch (const std::exception& e) {
        std::cout << "   ✗ Exception: " << e.what() << "\n";
    }

    std::cout << "\n=== ALL INFRASTRUCTURE READY ===\n\n";
}

/**
 * @brief Новый main() для V8Unpack с современной архитектурой
 */
int main(int argc, char* argv[]) {
    std::cout << "V8Unpack 2.0 - Modern Architecture\n";
    std::cout << "===================================\n";

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Для демонстрации инфраструктуры - запускаем тесты
        if (argc == 2 && std::string(argv[1]) == "--demo") {
            demoInfrastructure();
            return EXIT_SUCCESS;
        }

        // Создание основного приложения
        std::unique_ptr<Application> app;

        // TODO: Улучшение - добавить возможность кастомной конфигурации через AppFactory
        if (std::getenv("V8UNPACK_CUSTOM_CONFIG")) {
            // app = ApplicationFactory::createWithComponents(/* custom config */);
            std::cout << "Using custom configuration...\n";
        } else {
            app = ApplicationFactory::createDefault();
        }

        if (!app || !app->isInitialized()) {
            std::cerr << "Critical error: Failed to initialize V8Unpack application\n";
            return EXIT_FAILURE;
        }

        // Обрабатываем аргументы командной строки через новый Application
        int result = app->run(argc, argv);

        // Вычисляем время выполнения
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

        std::cout << "\nExecution completed in " << duration.count() << "ms "
                  << "with result: " << result << "\n";

        return result;

    } catch (const std::exception& e) {
        std::cerr << "\nFatal exception: " << e.what() << "\n";

        // TODO: Улучшение - добавить crash reporting
        // CrashReporter::reportException(e);

        return EXIT_FAILURE;

    } catch (...) {
        std::cerr << "\nUnknown fatal error occurred\n";
        return EXIT_FAILURE;
    }
}

/**
 * @brief РОАДМАП ИНТЕГРАЦИИ
 *
 * Этот файл служит мостом между старой и новой архитектурой.
 * Полная интеграция выполняется поэтапно:
 *
 * ЭТАП 1: Infrastructure Integration (Текущий статус)
 * - [x] Создать новый main.cpp с архитектурой
 * - [x] Подключить Application, ArgumentParser, MetadataAnalyzer
 * - [x] Обеспечить совместимость с существующим ./v8unpack
 * - [ ] Интегрировать BatchProcessor для --list опций
 *
 * ЭТАП 2: Command System Integration (Следующий)
 * - [ ] Создать V8CommandFactory для реальных V8Unpack команд
 * - [ ] Интегрировать unpack, pack, list, parse команды
 * - [ ] Обеспечить fallback на legacy функции
 * - [ ] Протестировать все команды через новый Application
 *
 * ЭТАП 3: Legacy Removal (Финальный)
 * - [ ] Удалить старую логику main.cpp
 * - [ ] Убрать extern legacy функции
 * - [ ] Полная миграция на новую архитектуру
 * - [ ] Code cleanup и optimization
 *
 * ВЫГОДЫ НОВОЙ АРХИТЕКТУРЫ:
 * + Удобство поддержки и расширения
 * + Dependency Injection для тестирования
 * + Centralized error handling
 * + Metadata analysis capabilities
 * + Batch processing
 * + Better code organization
 * - Временные накладные расходы на интеграцию
 */
