#include "RegexRegistry.h"

// Включаем заголовок с GUID константами
#include "../guids.h"

#include <algorithm>
#include <stdexcept>

namespace v8unpack {

RegexRegistry::RegexRegistry()
    : START_REGEX(R"(\{)")
    , END_REGEX(R"([^}]*\})") {
    initializeGuidMap();
}

void RegexRegistry::initializeGuidMap() {
    // Основные метаданные
    guidMap_[MetadataRegexType::SUBSYSTEMS]                  = GUID_Subsystems;
    guidMap_[MetadataRegexType::COMMON_MODULES]              = GUID_CommonModules;
    guidMap_[MetadataRegexType::SESSION_PARAMETERS]          = GUID_SessionParameters;
    guidMap_[MetadataRegexType::ROLES]                       = GUID_Roles;
    guidMap_[MetadataRegexType::COMMON_ATTRIBUTES]           = GUID_CommonAttributes;
    guidMap_[MetadataRegexType::EXCHANGE_PLANS]              = GUID_ExchangePlans;
    guidMap_[MetadataRegexType::FILTER_CRITERIA]             = GUID_FilterCriteria;
    guidMap_[MetadataRegexType::EVENT_SUBSCRIPTIONS]         = GUID_EventSubscriptions;
    guidMap_[MetadataRegexType::SCHEDULED_JOBS]              = GUID_ScheduledJobs;
    guidMap_[MetadataRegexType::FUNCTIONAL_OPTIONS]          = GUID_FunctionalOptions;
    guidMap_[MetadataRegexType::FUNCTIONAL_OPTIONS_PARAMETERS] = GUID_FunctionalOptionsParameters;
    guidMap_[MetadataRegexType::DEFINED_TYPES]               = GUID_DefinedTypes;
    guidMap_[MetadataRegexType::SETTINGS_STORAGES]           = GUID_SettingsStorages;
    guidMap_[MetadataRegexType::COMMON_FORMS]                = GUID_CommonForms;
    guidMap_[MetadataRegexType::COMMON_COMMANDS]             = GUID_CommonCommands;
    guidMap_[MetadataRegexType::COMMAND_GROUPS]              = GUID_CommandGroups;
    guidMap_[MetadataRegexType::INTERFACES]                  = GUID_Interfaces;
    guidMap_[MetadataRegexType::COMMON_TEMPLATES]            = GUID_CommonTemplates;
    guidMap_[MetadataRegexType::COMMON_PICTURES]             = GUID_CommonPictures;
    guidMap_[MetadataRegexType::XDTO_PACKAGES]               = GUID_XDTOPackages;
    guidMap_[MetadataRegexType::WEB_SERVICES]                = GUID_WebServices;
    guidMap_[MetadataRegexType::HTTPS_SERVICES]              = GUID_HTTPServices;
    guidMap_[MetadataRegexType::WS_REFERENCES]               = GUID_WSReferences;
    guidMap_[MetadataRegexType::STYLE_ITEMS]                 = GUID_StyleItems;
    guidMap_[MetadataRegexType::STYLES]                      = GUID_Styles;
    guidMap_[MetadataRegexType::LANGUAGES]                   = GUID_Languages;

    // Конфигурационные объекты
    guidMap_[MetadataRegexType::CATALOGS]                    = GUID_Catalogs;
    guidMap_[MetadataRegexType::CONSTANTS]                   = GUID_Constants;
    guidMap_[MetadataRegexType::DOCUMENTS]                   = GUID_Documents;
    guidMap_[MetadataRegexType::NUMERATORS]                  = GUID_Numerators;
    guidMap_[MetadataRegexType::SEQUENCES]                   = GUID_Sequences;
    guidMap_[MetadataRegexType::DOCUMENT_JOURNALS]           = GUID_JournDocuments;
    guidMap_[MetadataRegexType::ENUMS]                       = GUID_Enums;
    guidMap_[MetadataRegexType::REPORTS]                     = GUID_Reports;
    guidMap_[MetadataRegexType::DATA_PROCESSORS]             = GUID_DataProcessors;
    guidMap_[MetadataRegexType::CHART_OF_CHARACTERISTIC_TYPES] = GUID_ChartOfCharacteristicTypes;
    guidMap_[MetadataRegexType::CHARTS_OF_ACCOUNTS]          = GUID_ChartsOfAccounts;
    guidMap_[MetadataRegexType::CHARTS_OF_CALCULATION_TYPES] = GUID_ChartsOfCalculationTypes;
    guidMap_[MetadataRegexType::INFORMATION_REGISTERS]       = GUID_InformationRegisters;
    guidMap_[MetadataRegexType::ACCUMULATION_REGISTERS]      = GUID_AccumulationRegisters;
    guidMap_[MetadataRegexType::ACCOUNTING_REGISTERS]        = GUID_AccountingRegisters;
    guidMap_[MetadataRegexType::CALCULATION_REGISTERS]       = GUID_CalculationRegisters;
    guidMap_[MetadataRegexType::BUSINESS_PROCESSES]          = GUID_BusinessProcesses;
    guidMap_[MetadataRegexType::TASKS]                       = GUID_Tasks;
    guidMap_[MetadataRegexType::EXTERNAL_DATA_SOURCES]       = GUID_ExternalDataSources;
}

const std::regex& RegexRegistry::getRegex(MetadataRegexType type) const {
    // Lazy initialization - создаем regex только при первом обращении
    if (regexCache_.find(type) == regexCache_.end()) {
        regexCache_[type] = createRegex(type);
    }
    return regexCache_.at(type);
}

bool RegexRegistry::isSupported(MetadataRegexType type) const {
    return guidMap_.find(type) != guidMap_.end();
}

std::string RegexRegistry::getGuid(MetadataRegexType type) const {
    auto it = guidMap_.find(type);
    if (it != guidMap_.end()) {
        return it->second;
    }
    throw std::invalid_argument("Unsupported metadata type");
}

std::regex RegexRegistry::createRegex(MetadataRegexType type) const {
    auto guidIt = guidMap_.find(type);
    if (guidIt == guidMap_.end()) {
        throw std::invalid_argument("Unsupported metadata type for regex creation");
    }

    // Создаем паттерн по аналогии с main.cpp: StartRegex + GUID + EndRegex
    std::string pattern = START_REGEX + guidIt->second + END_REGEX;
    return std::regex(pattern);
}

// ===== Реализация MetadataRegexUtils =====

MetadataRegexType MetadataRegexUtils::guidToType(const std::string& guidString) {
    // Обратная карта GUID -> тип (можно оптимизировать кэшированием)
    static std::unordered_map<std::string, MetadataRegexType> reverseMap;

    // Инициализация обратной карты при первом использовании
    if (reverseMap.empty()) {
        auto allTypes = getAllSupportedTypes();
        RegexRegistry registry;

        for (auto type : allTypes) {
            try {
                std::string guid = registry.getGuid(type);
                reverseMap[guid] = type;
            } catch (const std::exception&) {
                // Пропускаем неподдерживаемые типы
                continue;
            }
        }
    }

    auto it = reverseMap.find(guidString);
    if (it != reverseMap.end()) {
        return it->second;
    }

    throw std::invalid_argument("Unknown GUID: " + guidString);
}

bool MetadataRegexUtils::isKnownMetadataGuid(const std::string& guidString) {
    try {
        guidToType(guidString);
        return true;
    } catch (const std::invalid_argument&) {
        return false;
    }
}

std::vector<MetadataRegexType> MetadataRegexUtils::getAllSupportedTypes() {
    return {
        // Основные метаданные
        MetadataRegexType::SUBSYSTEMS,
        MetadataRegexType::COMMON_MODULES,
        MetadataRegexType::SESSION_PARAMETERS,
        MetadataRegexType::ROLES,
        MetadataRegexType::COMMON_ATTRIBUTES,
        MetadataRegexType::EXCHANGE_PLANS,
        MetadataRegexType::FILTER_CRITERIA,
        MetadataRegexType::EVENT_SUBSCRIPTIONS,
        MetadataRegexType::SCHEDULED_JOBS,
        MetadataRegexType::FUNCTIONAL_OPTIONS,
        MetadataRegexType::FUNCTIONAL_OPTIONS_PARAMETERS,
        MetadataRegexType::DEFINED_TYPES,
        MetadataRegexType::SETTINGS_STORAGES,
        MetadataRegexType::COMMON_FORMS,
        MetadataRegexType::COMMON_COMMANDS,
        MetadataRegexType::COMMAND_GROUPS,
        MetadataRegexType::INTERFACES,
        MetadataRegexType::COMMON_TEMPLATES,
        MetadataRegexType::COMMON_PICTURES,
        MetadataRegexType::XDTO_PACKAGES,
        MetadataRegexType::WEB_SERVICES,
        MetadataRegexType::HTTPS_SERVICES,
        MetadataRegexType::WS_REFERENCES,
        MetadataRegexType::STYLE_ITEMS,
        MetadataRegexType::STYLES,
        MetadataRegexType::LANGUAGES,

        // Конфигурационные объекты
        MetadataRegexType::CATALOGS,
        MetadataRegexType::CONSTANTS,
        MetadataRegexType::DOCUMENTS,
        MetadataRegexType::NUMERATORS,
        MetadataRegexType::SEQUENCES,
        MetadataRegexType::DOCUMENT_JOURNALS,
        MetadataRegexType::ENUMS,
        MetadataRegexType::REPORTS,
        MetadataRegexType::DATA_PROCESSORS,
        MetadataRegexType::CHART_OF_CHARACTERISTIC_TYPES,
        MetadataRegexType::CHARTS_OF_ACCOUNTS,
        MetadataRegexType::CHARTS_OF_CALCULATION_TYPES,
        MetadataRegexType::INFORMATION_REGISTERS,
        MetadataRegexType::ACCUMULATION_REGISTERS,
        MetadataRegexType::ACCOUNTING_REGISTERS,
        MetadataRegexType::CALCULATION_REGISTERS,
        MetadataRegexType::BUSINESS_PROCESSES,
        MetadataRegexType::TASKS,
        MetadataRegexType::EXTERNAL_DATA_SOURCES
    };
}

} // namespace v8unpack
