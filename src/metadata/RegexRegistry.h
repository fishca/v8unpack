#ifndef METADATA_REGEX_REGISTRY_H
#define METADATA_REGEX_REGISTRY_H

#include <string>
#include <regex>
#include <unordered_map>

namespace v8unpack {

/**
 * @brief Типы метаданных 1C для которых нужны регулярные выражения
 *
 * Перечисление соответствует константам GUID_* из guids.h
 */
enum class MetadataRegexType {
    // Основные метаданные
    SUBSYSTEMS,
    COMMON_MODULES,
    SESSION_PARAMETERS,
    ROLES,
    COMMON_ATTRIBUTES,
    EXCHANGE_PLANS,
    FILTER_CRITERIA,
    EVENT_SUBSCRIPTIONS,
    SCHEDULED_JOBS,
    FUNCTIONAL_OPTIONS,
    FUNCTIONAL_OPTIONS_PARAMETERS,
    DEFINED_TYPES,
    SETTINGS_STORAGES,
    COMMON_FORMS,
    COMMON_COMMANDS,
    COMMAND_GROUPS,
    INTERFACES,
    COMMON_TEMPLATES,
    COMMON_PICTURES,
    XDTO_PACKAGES,
    WEB_SERVICES,
    HTTPS_SERVICES,
    WS_REFERENCES,
    STYLE_ITEMS,
    STYLES,
    LANGUAGES,

    // Конфигурационные объекты
    CATALOGS,
    CONSTANTS,
    DOCUMENTS,
    NUMERATORS,
    SEQUENCES,
    DOCUMENT_JOURNALS,
    ENUMS,
    REPORTS,
    DATA_PROCESSORS,
    CHART_OF_CHARACTERISTIC_TYPES,
    CHARTS_OF_ACCOUNTS,
    CHARTS_OF_CALCULATION_TYPES,
    INFORMATION_REGISTERS,
    ACCUMULATION_REGISTERS,
    ACCOUNTING_REGISTERS,
    CALCULATION_REGISTERS,
    BUSINESS_PROCESSES,
    TASKS,
    EXTERNAL_DATA_SOURCES
};

/**
 * @brief Централизованное управление регулярными выражениями для метаданных 1C
 *
 * Заменяет глобальные rx_* переменные из main.cpp централизованным управлением.
 * Создает regex объекты по требованию для поиска типов метаданных в текстовых файлах.
 *
 * Паттерн: Factory + Registry
 */
class RegexRegistry {
public:
    /**
     * @brief Конструктор
     *
     * Инициализирует базовые паттерны (StartRegex/EndRegex)
     */
    RegexRegistry();

    /**
     * @brief Деструктор
     */
    ~RegexRegistry() = default;

    /**
     * @brief Получить регулярное выражение для типа метаданных
     *
     * @param type Тип метаданных
     * @return std::regex объект для поиска
     */
    const std::regex& getRegex(MetadataRegexType type) const;

    /**
     * @brief Проверить соответствует ли тип метаданных доступным
     *
     * @param type Тип метаданных
     * @return true если тип поддерживается
     */
    bool isSupported(MetadataRegexType type) const;

    /**
     * @brief Получить строковое представление GUID для типа
     *
     * @param type Тип метаданных
     * @return строка GUID
     */
    std::string getGuid(MetadataRegexType type) const;

private:
    /**
     * @brief Инициализация карты GUID для типов метаданных
     */
    void initializeGuidMap();

    /**
     * @brief Создание regex для конкретного типа
     *
     * @param type Тип метаданных
     * @return std::regex объект
     */
    std::regex createRegex(MetadataRegexType type) const;

private:
    // Базовые паттерны
    const std::string START_REGEX;  // R"(\{)"
    const std::string END_REGEX;    // R"([^}]*\})"

    // Карта GUID для типов
    mutable std::unordered_map<MetadataRegexType, std::string> guidMap_;

    // Кэш regex объектов (mutable для lazy initialization)
    mutable std::unordered_map<MetadataRegexType, std::regex> regexCache_;
};

/**
 * @brief Вспомогательные функции для работы с метаданными
 */
class MetadataRegexUtils {
public:
    /**
     * @brief Преобразовать строковый GUID в тип
     *
     * @param guidString GUID строка
     * @return MetadataRegexType или исключение если не найден
     */
    static MetadataRegexType guidToType(const std::string& guidString);

    /**
     * @brief Проверить является ли GUID известным типом метаданных
     *
     * @param guidString GUID для проверки
     * @return true если GUID соответствует известному типу
     */
    static bool isKnownMetadataGuid(const std::string& guidString);

    /**
     * @brief Получить список всех поддерживаемых типов
     *
     * @return вектор всех поддерживаемых MetadataRegexType
     */
    static std::vector<MetadataRegexType> getAllSupportedTypes();
};

} // namespace v8unpack

#endif // METADATA_REGEX_REGISTRY_H
