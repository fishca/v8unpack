#ifndef METADATA_METADATA_ANALYZER_H
#define METADATA_METADATA_ANALYZER_H

#include <string>
#include <vector>
#include <memory>
#include <map>

#include "../utils/ErrorCodes.h"
#include "RegexRegistry.h"

namespace v8unpack {

/**
 * @brief Результат анализа мета-данных конфигурации 1C
 */
struct MetadataAnalysisResult {
    /**
     * @brief Версия конфигурации из файла version
     */
    std::string configVersion;

    /**
     * @brief Найденные GUID'ы корневых метаданных
     */
    std::vector<std::string> foundTypeGuids;

    /**
     * @brief Распарсенные имена типов (например, "Справочники", "Документы")
     */
    std::map<std::string, std::string> guidToNameMapping;

    /**
     * @brief Статистика анализа
     */
    struct AnalysisStats {
        size_t totalObjects = 0;      // Всего найдено объектов
        size_t validTypes = 0;        // Валидных типов
        size_t warnings = 0;          // Предупреждений
        size_t errors = 0;            // Ошибок
    } stats;

    /**
     * @brief Успешность анализа
     */
    bool valid = false;

    /**
     * @brief Подробная информация об ошибках
     */
    std::vector<ErrorInfo> errors;

    // Вспомогательные методы
    bool hasErrors() const { return !errors.empty(); }
    size_t getErrorCount() const { return errors.size(); }
    std::string getFirstError() const {
        return errors.empty() ? "" : errors[0].message();
    }
};

/**
 * @brief Анализатор мета-данных конфигураций 1C
 *
 * Отвечает за анализ и извлечение информации из мета-данных V8:
 * - Версия конфигурации
 * - Структура метаданных (справочники, документы, регистры и т.д.)
 * - Валидация форматов
 *
 * Использует RegexRegistry для поиска шаблонов в текстовых файлах.
 */
class MetadataAnalyzer {
public:
    /**
     * @brief Конструктор
     */
    MetadataAnalyzer();

    /**
     * @brief Деструктор
     */
    ~MetadataAnalyzer() = default;

    /**
     * @brief Основной метод анализа файла с мета-данными
     *
     * @param filePath Путь к файлу с мета-данными (.metadata или root файл)
     * @return Результат анализа
     */
    MetadataAnalysisResult analyze(const std::string& filePath);

    /**
     * @brief Анализ извлеченного содержимого мета-данных
     *
     * @param metadataContent Содержимое файла с мета-данными
     * @param sourceName Имя источника для сообщений об ошибках
     * @return Результат анализа
     */
    MetadataAnalysisResult analyzeContent(const std::string& metadataContent,
                                        const std::string& sourceName = "content");

    /**
     * @brief Проверка валидности GUID мета-данных
     *
     * @param guid GUID для проверки
     * @return true если GUID соответствует известному типу
     */
    bool isValidMetadataGuid(const std::string& guid) const;

    /**
     * @brief Получение имени типа по GUID
     *
     * @param guid GUID типа
     * @return Имя типа или пустую строку если неизвестен
     */
    std::string getTypeName(const std::string& guid) const;

private:
    /**
     * @brief Проверка существования и доступности файла
     */
    ErrorInfo validateFile(const std::string& filePath) const;

    /**
     * @brief Чтение содержимого файла
     */
    std::string readFileContent(const std::string& filePath, ErrorInfo& error) const;

    /**
     * @brief Извлечение версии конфигурации из version файла
     */
    std::string extractConfigVersion(const std::string& content) const;

    /**
     * @brief Основная логика анализа содержимого
     */
    MetadataAnalysisResult performAnalysis(const std::string& content,
                                         const std::string& sourceName);

    /**
     * @brief Поиск всех GUID'ов метаданных в содержимом
     */
    std::vector<std::string> findAllMetadataGuids(const std::string& content);

    /**
     * @brief Валидация найденного GUID
     */
    bool validateGuid(const std::string& guid) const;

    /**
     * @brief Создание mapping GUID -> имя типа
     */
    std::map<std::string, std::string> createGuidMappings(const std::vector<std::string>& guids);

    /**
     * @brief Расчет статистики анализа
     */
    MetadataAnalysisResult::AnalysisStats calculateStatistics(
        const std::vector<std::string>& guids,
        const std::vector<ErrorInfo>& errors) const;

    /**
     * @brief Разбор структуры корневых данных (root file analysis)
     */
    ErrorInfo parseRootStructure(const std::string& content,
                               MetadataAnalysisResult& result);

    /**
     * @brief Разбор содержимого метаданных
     */
    ErrorInfo parseMetadataContent(const std::string& content,
                                 MetadataAnalysisResult& result);

private:
    /**
     * @brief Реестр регулярных выражений для поиска
     */
    std::shared_ptr<RegexRegistry> regexRegistry_;

    /**
     * @brief Кэш для имен типов (ленивая инициализация)
     */
    mutable std::map<std::string, std::string> typeNameCache_;
};

/**
 * @brief Утилиты для работы с мета-данными
 */
class MetadataUtils {
public:
    /**
     * @brief Проверка является ли файл файлом метаданных
     */
    static bool isMetadataFile(const std::string& filePath);

    /**
     * @brief Извлечение имени файла с метаданными из пути конфигурации
     */
    static std::string getMetadataFilePath(const std::string& configPath);

    /**
     * @brief Форматирование результатов анализа в читаемую строку
     */
    static std::string formatAnalysisResult(const MetadataAnalysisResult& result);

    /**
     * @brief Создание краткого отчета анализа
     */
    static std::string createAnalysisReport(const MetadataAnalysisResult& result);
};

} // namespace v8unpack

#endif // METADATA_METADATA_ANALYZER_H
