#include "MetadataAnalyzer.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#include <boost/filesystem.hpp>

#include "../utils/StringConverters.h"

namespace fs = boost::filesystem;

namespace v8unpack {

MetadataAnalyzer::MetadataAnalyzer()
    : regexRegistry_(std::make_shared<RegexRegistry>()) {
    // Конструктор пока пустой
}

MetadataAnalysisResult MetadataAnalyzer::analyze(const std::string& filePath) {
    MetadataAnalysisResult result;

    // Проверяем файл
    ErrorInfo fileError = validateFile(filePath);
    if (fileError.isSuccess()) {
        // Файл валиден, читаем содержимое
        ErrorInfo readError;
        std::string content = readFileContent(filePath, readError);

        if (readError.isSuccess() && !content.empty()) {
            // Выполняем анализ содержимого
            result = performAnalysis(content, filePath);
        } else {
            result.errors.push_back(readError);
            result.valid = false;
        }
    } else {
        result.errors.push_back(fileError);
        result.valid = false;
    }

    return result;
}

MetadataAnalysisResult MetadataAnalyzer::analyzeContent(const std::string& metadataContent,
                                                      const std::string& sourceName) {
    return performAnalysis(metadataContent, sourceName);
}

bool MetadataAnalyzer::isValidMetadataGuid(const std::string& guid) const {
    return MetadataRegexUtils::isKnownMetadataGuid(guid);
}

std::string MetadataAnalyzer::getTypeName(const std::string& guid) const {
    // Ленивая инициализация кэша
    if (typeNameCache_.empty()) {
        // Инициализируем кэш всеми известными GUID->имя
        auto allTypes = MetadataRegexUtils::getAllSupportedTypes();
        for (auto type : allTypes) {
            try {
                std::string typeGuid = regexRegistry_->getGuid(type);
                // Здесь нужно получить имя типа - это будет реализовано позже
                // Пока используем заглушки
                typeNameCache_[typeGuid] = std::to_string(static_cast<int>(type));
            } catch (const std::exception&) {
                // Пропускаем неизвестные типы
            }
        }
    }

    auto it = typeNameCache_.find(guid);
    return (it != typeNameCache_.end()) ? it->second : "";
}

ErrorInfo MetadataAnalyzer::validateFile(const std::string& filePath) const {
    if (filePath.empty()) {
        return ErrorInfo(ErrorCode::INVALID_ARGUMENTS, "File path is empty");
    }

    if (!fs::exists(filePath)) {
        return ErrorInfo(ErrorCode::FILE_NOT_FOUND, "File not found", "", filePath);
    }

    if (!fs::is_regular_file(filePath)) {
        return ErrorInfo(ErrorCode::INVALID_FILE_PATH, "Path is not a regular file", "", filePath);
    }

    // Проверяем права доступа
    try {
        std::ifstream testFile(filePath);
        if (!testFile.is_open()) {
            return ErrorInfo(ErrorCode::FILE_ACCESS_DENIED, "Cannot open file for reading", "", filePath);
        }
        testFile.close();
    } catch (const std::exception& e) {
        return ErrorInfo(ErrorCode::FILE_ACCESS_DENIED,
                        std::string("File access error: ") + e.what(), "", filePath);
    }

    return ErrorInfo(ErrorCode::SUCCESS);
}

std::string MetadataAnalyzer::readFileContent(const std::string& filePath, ErrorInfo& error) const {
    try {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            error = ErrorInfo(ErrorCode::FILE_READ_ERROR, "Cannot open file", "", filePath);
            return "";
        }

        // Читаем весь файл
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());

        file.close();

        // Проверяем что файл не пустой
        if (content.empty()) {
            error = ErrorInfo(ErrorCode::FILE_READ_ERROR, "File is empty", "", filePath);
            return "";
        }

        return content;
    } catch (const std::exception& e) {
        error = ErrorInfo(ErrorCode::FILE_READ_ERROR,
                         std::string("Read error: ") + e.what(), "", filePath);
        return "";
    }
}

std::string MetadataAnalyzer::extractConfigVersion(const std::string& content) const {
    // Простая экстракция версии - в реальности это будет сложнее
    // TODO: Реализовать правильный парсинг version файла
    return "8.3.0.0"; // Заглушка
}

MetadataAnalysisResult MetadataAnalyzer::performAnalysis(const std::string& content,
                                                       const std::string& sourceName) {
    MetadataAnalysisResult result;
    result.valid = true; // Оптимистично начинаем

    // Находим все GUID'ы в содержимом
    auto foundGuids = findAllMetadataGuids(content);
    result.foundTypeGuids = foundGuids;

    // Проверяем каждый GUID
    std::vector<ErrorInfo> analysisErrors;
    for (const auto& guid : foundGuids) {
        if (!validateGuid(guid)) {
            analysisErrors.push_back(
                ErrorInfo(ErrorCode::METADATA_PARSING_ERROR,
                         "Invalid metadata GUID found", guid, sourceName)
            );
        }
    }

    // Создаем mapping GUID -> имя типа
    result.guidToNameMapping = createGuidMappings(foundGuids);

    // Извлекаем версию конфигурации
    result.configVersion = extractConfigVersion(content);

    // Дополнительный анализ структуры (если это root файл)
    ErrorInfo structureError = parseRootStructure(content, result);
    if (!structureError.isSuccess()) {
        analysisErrors.push_back(structureError);
    }

    // Анализируем содержимое метаданных
    ErrorInfo contentError = parseMetadataContent(content, result);
    if (!contentError.isSuccess()) {
        analysisErrors.push_back(contentError);
    }

    // Финализируем результат
    result.errors = analysisErrors;
    result.stats = calculateStatistics(foundGuids, analysisErrors);
    result.valid = analysisErrors.empty();

    return result;
}

std::vector<std::string> MetadataAnalyzer::findAllMetadataGuids(const std::string& content) {
    std::vector<std::string> guids;

    // Используем все известные типы для поиска
    auto allTypes = MetadataRegexUtils::getAllSupportedTypes();

    for (auto type : allTypes) {
        try {
            const std::regex& regex = regexRegistry_->getRegex(type);
            std::smatch matches;

            // Ищем все совпадения в содержимом
            auto searchStart = content.cbegin();
            while (std::regex_search(searchStart, content.cend(), matches, regex)) {
                // matches[1] должен содержать GUID (вторая группа захвата)
                if (matches.size() > 1) {
                    std::string foundGuid = matches[1].str();
                    if (!foundGuid.empty() && std::find(guids.begin(), guids.end(), foundGuid) == guids.end()) {
                        guids.push_back(foundGuid);
                    }
                }

                // Продолжаем поиск после текущего совпадения
                searchStart = matches.suffix().first;
            }
        } catch (const std::exception&) {
            // Пропускаем regex ошибки для отдельных типов
            continue;
        }
    }

    return guids;
}

bool MetadataAnalyzer::validateGuid(const std::string& guid) const {
    // Простая проверка формата GUID (xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx)
    if (guid.length() != 36) return false;

    for (size_t i = 0; i < guid.length(); ++i) {
        char c = guid[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') return false;
        } else {
            if (!std::isxdigit(c)) return false;
        }
    }

    return true;
}

std::map<std::string, std::string> MetadataAnalyzer::createGuidMappings(const std::vector<std::string>& guids) {
    std::map<std::string, std::string> mappings;

    for (const auto& guid : guids) {
        std::string typeName = getTypeName(guid);
        if (!typeName.empty()) {
            mappings[guid] = typeName;
        } else {
            mappings[guid] = "Unknown"; // Заглушка для неизвестных типов
        }
    }

    return mappings;
}

MetadataAnalysisResult::AnalysisStats MetadataAnalyzer::calculateStatistics(
    const std::vector<std::string>& guids,
    const std::vector<ErrorInfo>& errors) const {

    MetadataAnalysisResult::AnalysisStats stats;

    stats.totalObjects = guids.size();

    // Подсчитываем валидные типы
    for (const auto& guid : guids) {
        if (isValidMetadataGuid(guid)) {
            stats.validTypes++;
        }
    }

    // Подсчитываем ошибки и предупреждения
    stats.errors = 0;
    stats.warnings = 0;

    for (const auto& error : errors) {
        if (error.code() == ErrorCode::METADATA_PARSING_ERROR) {
            stats.errors++;
        } else {
            stats.warnings++;
        }
    }

    return stats;
}

ErrorInfo MetadataAnalyzer::parseRootStructure(const std::string& content,
                                             MetadataAnalysisResult& result) {
    // TODO: Реализовать анализ структуры root файла
    // Пока возвращаем успех
    return ErrorInfo(ErrorCode::SUCCESS);
}

ErrorInfo MetadataAnalyzer::parseMetadataContent(const std::string& content,
                                               MetadataAnalysisResult& result) {
    // TODO: Реализовать анализ содержимого метаданных
    // Пока возвращаем успех
    return ErrorInfo(ErrorCode::SUCCESS);
}

// =========== Реализация MetadataUtils ===========

bool MetadataUtils::isMetadataFile(const std::string& filePath) {
    std::string lowerPath = filePath;
    std::transform(lowerPath.begin(), lowerPath.end(), lowerPath.begin(), ::tolower);

    return lowerPath.find("metadata") != std::string::npos ||
           lowerPath.find("root") != std::string::npos;
}

std::string MetadataUtils::getMetadataFilePath(const std::string& configPath) {
    // TODO: Реализовать логику определения пути к файлу метаданных
    return configPath + "/metadata"; // Заглушка
}

std::string MetadataUtils::formatAnalysisResult(const MetadataAnalysisResult& result) {
    std::stringstream ss;

    ss << "=== Metadata Analysis Result ===\n";
    ss << "Configuration Version: " << result.configVersion << "\n";
    ss << "Analysis Valid: " << (result.valid ? "Yes" : "No") << "\n";
    ss << "\n";

    ss << "Statistics:\n";
    ss << "- Total Objects: " << result.stats.totalObjects << "\n";
    ss << "- Valid Types: " << result.stats.validTypes << "\n";
    ss << "- Warnings: " << result.stats.warnings << "\n";
    ss << "- Errors: " << result.stats.errors << "\n";
    ss << "\n";

    if (!result.foundTypeGuids.empty()) {
        ss << "Found Metadata Types:\n";
        for (const auto& guid : result.foundTypeGuids) {
            auto it = result.guidToNameMapping.find(guid);
            std::string name = (it != result.guidToNameMapping.end()) ? it->second : "Unknown";
            ss << "- " << guid << " -> " << name << "\n";
        }
        ss << "\n";
    }

    if (!result.errors.empty()) {
        ss << "Errors:\n";
        for (const auto& error : result.errors) {
            ss << "- " << error.message();
            if (!error.details().empty()) {
                ss << " (" << error.details() << ")";
            }
            ss << "\n";
        }
    }

    return ss.str();
}

std::string MetadataUtils::createAnalysisReport(const MetadataAnalysisResult& result) {
    std::stringstream ss;

    ss << "Configuration Analysis Report\n";
    ss << "===========================\n\n";

    ss << "Summary:\n";
    ss << "- Config Version: " << result.configVersion << "\n";
    ss << "- Objects Found: " << result.stats.totalObjects << "\n";
    ss << "- Validation: " << (result.valid ? "PASSED" : "FAILED") << "\n\n";

    if (!result.errors.empty()) {
        ss << "Issues Found:\n";
        for (const auto& error : result.errors) {
            ss << "* " << error.message() << "\n";
        }
        ss << "\n";
    }

    ss << "Recommendations:\n";
    if (result.stats.errors > 0) {
        ss << "- Review and fix metadata parsing errors\n";
    }
    if (result.stats.validTypes == 0) {
        ss << "- No valid metadata types found - check file format\n";
    }
    if (result.configVersion.empty()) {
        ss << "- Configuration version not detected\n";
    }

    return ss.str();
}

} // namespace v8unpack
