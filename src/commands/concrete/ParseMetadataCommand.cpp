#include "ParseMetadataCommand.h"

#include "../../parse_tree.h"
#include "../../common.h"
#include "../../guids.h"
#include "../../ConfigStructureParser.h"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <algorithm>

namespace fs = boost::filesystem;

namespace v8unpack {

ParseMetadataCommand::ParseMetadataCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger), metadataAnalyzer_(std::make_shared<MetadataAnalyzer>()) {
    initializeCategoryMappings();
}

int ParseMetadataCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputDir = args[0];
    std::string outputDir = args[1];

    if (logger_) {
        logger_->Status("[PARSE_METADATA] Начало классификации метаданных...");
    }

    std::cout << "Классификация файлов из каталога '" << inputDir << "' в '" << outputDir << "'" << std::endl;

    try {
        // Сканируем входной каталог
        std::vector<fs::path> files = scanDirectory(inputDir);
        if (files.empty()) {
            std::cerr << "В каталоге '" << inputDir << "' не найдено файлов для классификации" << std::endl;
            return 1;
        }

        std::cout << "Найдено " << files.size() << " файлов для классификации" << std::endl;

        // Обрабатываем корневой файл конфигурации и извлекаем GUID конфигурации
        std::string rootGuid = processRootConfigurationFile(inputDir, outputDir);
        if (rootGuid.empty()) {
            std::cerr << "Ошибка обработки корневого файла конфигурации" << std::endl;
            return 1;
        }

        // Парсим корневой файл конфигурации для извлечения GUID'ов файлов
        fs::path rootConfigFilePath = fs::path(outputDir) / "Конфигурация" / rootGuid;
        parseRootConfigForFileGuids(rootConfigFilePath);

        // Шаг 2: Обрабатываем остальные файлы
        std::cout << "Обрабатываем остальные файлы..." << std::endl;

        for (const auto& filePath : files) {
            std::string filename = filePath.filename().string();

            // Пропускаем корневой файл конфигурации (он уже обработан)
            if (filename == rootGuid) {
                continue;
            }

            std::string categoryName = classifyFile(filePath);

            if (!categoryName.empty()) {
                bool success = copyFileToCategory(filePath, categoryName, outputDir, inputDir);
                if (success) {
                    categoryFiles_[categoryName].push_back(filePath.filename().string());
                    std::cout << "✓ " << filePath.filename().string() << " → " << categoryName << std::endl;
                } else {
                    std::cout << "✗ Ошибка копирования: " << filePath.filename().string() << std::endl;
                }
            } else {
                // Копируем неклассифицированные файлы в каталог "Неопределено"
                bool success = copyFileToCategory(filePath, "Неопределено", outputDir, inputDir);
                if (success) {
                    categoryFiles_["Неопределено"].push_back(filePath.filename().string());
                    std::cout << "? Не удалось классифицировать: " << filePath.filename().string() << " → Неопределено" << std::endl;
                } else {
                    std::cout << "? Ошибка копирования в Неопределено: " << filePath.filename().string() << std::endl;
                }
            }
        }

        // Выводим отчёт
        printClassificationReport(categoryFiles_);

        if (logger_) {
            logger_->Status("[PARSE_METADATA] Классификация завершена успешно");
        }

        return 0;

    } catch (const std::exception& ex) {
        std::cerr << "Ошибка классификации: " << ex.what() << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка классификации метаданных", "exception", ex.what());
        }
        return 1;
    }
}

std::string ParseMetadataCommand::getName() const {
    return "parsemetadata";
}

std::string ParseMetadataCommand::getDescription() const {
    return "Классифицировать и организовать файлы метаданных по типам";
}

void ParseMetadataCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: parsemetadata" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  parsemetadata <input_dir> <output_dir>" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_dir   - путь к каталогу с распарсенными файлами (после команды parse)" << std::endl;
    std::cout << "  output_dir  - путь к выходному каталогу CFSRC для организованных метаданных" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  parsemetadata test_data_source/ CFSRC/" << std::endl;
    std::cout << std::endl;
}

std::vector<fs::path> ParseMetadataCommand::scanDirectory(const std::string& inputDir) {
    std::vector<fs::path> files;

    if (!fs::exists(inputDir)) {
        throw std::runtime_error("Каталог '" + inputDir + "' не существует");
    }

    if (!fs::is_directory(inputDir)) {
        throw std::runtime_error("'" + inputDir + "' не является каталогом");
    }

    // Рекурсивный обход каталога
    for (fs::recursive_directory_iterator it(inputDir), end; it != end; ++it) {
        if (fs::is_regular_file(it->path())) {
            files.push_back(it->path());
        }
    }

    return files;
}

std::string ParseMetadataCommand::classifyFile(const fs::path& filePath) {
    // Сначала проверяем специальные файлы
    std::string specialCategory = getSpecialFileCategory(filePath);
    if (!specialCategory.empty()) {
        return specialCategory;
    }

    // Извлекаем GUID из имени файла (убираем расширения типа .0, .1c и т.д.)
    std::string filename = filePath.filename().string();
    std::string guid = extractGuidFromFilename(filename);

    // Проверяем, есть ли GUID файла в мапе из корневого файла конфигурации
    auto fileGuidIt = fileGuidToCategoryMap_.find(guid);
    if (fileGuidIt != fileGuidToCategoryMap_.end() && fileGuidIt->second != std::string((const char*)md_Languages)) {
        return fileGuidIt->second;
    }

    // Читаем и парсим содержимое файла
    std::string content;
    try {
        std::ifstream file(filePath.string(), std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        content.assign((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            return "";
        }

        // Парсим как скобочную структуру 1C
        tree* parsedTree = parse_1Ctext(content, filePath.string());
        if (!parsedTree) {
            // Если не удалось распарсить, проверить на язык
            if (isLanguageFile(content)) {
                return std::string((const char*)md_Languages);
            }
            return "";
        }

        // Извлекаем GUID'ы из дерева
        std::vector<std::string> guids = extractGuidsFromTree(parsedTree);

        // Освобождаем память
        delete parsedTree;

        if (guids.empty()) {
            // Если GUID'ы не найдены, проверить на язык
            if (isLanguageFile(content)) {
                return std::string((const char*)md_Languages);
            }
            return "";
        }

        // Ищем основной GUID файла (обычно первый в списке)
        for (const auto& guid : guids) {
            std::string category = guidToCategoryName(guid);
            if (!category.empty()) {
                return category;
            }
        }

    } catch (const std::exception&) {
        // Игнорируем ошибки парсинга отдельных файлов
    }

    // Если не найдено по GUID, проверить на язык по содержимому
    if (isLanguageFile(content)) {
        return std::string((const char*)md_Languages);
    }

    return ""; // Не удалось классифицировать
}

std::vector<std::string> ParseMetadataCommand::extractGuidsFromTree(tree* root) {
    std::vector<std::string> guids;
    if (root) {
        traverseTreeForGuids(root, guids);
    }
    return guids;
}

void ParseMetadataCommand::traverseTreeForGuids(tree* node, std::vector<std::string>& guids) {
    if (!node) return;

    // Проверяем тип текущего узла - guid
    if (node->get_type() == node_type::nd_guid) {
        std::string guidValue = node->get_value().c_str();
        if (!guidValue.empty() && std::find(guids.begin(), guids.end(), guidValue) == guids.end()) {
            guids.push_back(guidValue);
        }
    }

    // Рекурсивно обходим дочерние узлы
    for (int i = 0; i < node->get_num_subnode(); ++i) {
        tree* child = node->get_subnode(i);
        traverseTreeForGuids(child, guids);
    }
}

std::string ParseMetadataCommand::guidToCategoryName(const std::string& guid) {
    auto it = guidToCategoryMap_.find(guid);
    return (it != guidToCategoryMap_.end()) ? it->second : "";
}

bool ParseMetadataCommand::createDirectoryStructure(const std::string& outputDir) {
    try {
        fs::path outputPath(outputDir);

        // Создаём основной каталог
        if (!fs::exists(outputPath)) {
            fs::create_directories(outputPath);
        }

        // Создаём подкаталоги для всех известных категорий
        for (const auto& pair : guidToCategoryMap_) {
            fs::path categoryDir = outputPath / pair.second;
            if (!fs::exists(categoryDir)) {
                fs::create_directories(categoryDir);
            }
        }

        // Специальные каталоги
        std::vector<std::string> specialDirs = {"Конфигурация", "Неопределено"};
        for (const auto& dirName : specialDirs) {
            fs::path specialDir = outputPath / dirName;
            if (!fs::exists(specialDir)) {
                fs::create_directories(specialDir);
            }
        }

        return true;

    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Ошибка создания структуры каталогов: " << ex.what() << std::endl;
        return false;
    }
}

bool ParseMetadataCommand::copyFileToCategory(const fs::path& sourceFile,
                                             const std::string& categoryName,
                                             const std::string& outputDir,
                                             const std::string& inputDir) {
    try {
        fs::path destDir = fs::path(outputDir) / categoryName;

        // Получаем относительный путь файла относительно входного каталога
        fs::path relativePath = fs::relative(sourceFile, fs::path(inputDir));
        fs::path destFile = destDir / relativePath;

        // Создаём каталоги если необходимо
        fs::path destDirectory = destFile.parent_path();
        if (!fs::exists(destDirectory)) {
            fs::create_directories(destDirectory);
        }

        fs::copy_file(sourceFile, destFile, fs::copy_options::overwrite_existing);
        return true;

    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Ошибка копирования файла '" << sourceFile.string() << "': " << ex.what() << std::endl;
        return false;
    }
}

std::string ParseMetadataCommand::getSpecialFileCategory(const fs::path& filePath) {
    std::string filename = filePath.filename().string();
    std::transform(filename.begin(), filename.end(), filename.begin(), ::tolower);

    if (filename == "root") return "Конфигурация";
    if (filename == "version") return "Конфигурация";
    if (filename == "versions") return "Конфигурация";

    return "";
}

void ParseMetadataCommand::printClassificationReport(const std::map<std::string, std::vector<std::string>>& categoryStats) {
    std::cout << std::endl;
    std::cout << "=== Отчёт о классификации метаданных ===" << std::endl;
    std::cout << std::endl;

    size_t totalFiles = 0;
    for (const auto& category : categoryStats) {
        totalFiles += category.second.size();
    }

    std::cout << "Всего обработано файлов: " << totalFiles << std::endl;
    std::cout << "Категорий создано: " << categoryStats.size() << std::endl;
    std::cout << std::endl;

    std::cout << "Распределение по категориям:" << std::endl;
    for (const auto& category : categoryStats) {
        std::cout << "  " << category.first << ": " << category.second.size() << " файлов" << std::endl;
    }

    std::cout << std::endl;
}

std::string ParseMetadataCommand::extractGuidFromFilename(const std::string& filename) {
    // Найти первую точку и взять часть до неё
    size_t dotPos = filename.find('.');
    if (dotPos != std::string::npos) {
        return filename.substr(0, dotPos);
    }
    return filename;
}

bool ParseMetadataCommand::isLanguageFile(const std::string& content) {
    // Файлы языков содержат названия языков в кавычках
    // Примеры: "Русский", "English", "Deutsch" и т.д.
    std::vector<std::string> languageNames = {
        "\"Русский\"", "\"English\"", "\"Deutsch\"", "\"Français\"",
        "\"Español\"", "\"Italiano\"", "\"Português\"", "\"中文\"",
        "\"日本語\"", "\"한국어\"", "\"Türkçe\"", "\"العربية\"",
        "\"हिन्दी\"", "\"বাংলা\"", "\"தமிழ்\"", "\"اردو\""
    };

    for (const auto& langName : languageNames) {
        if (content.find(langName) != std::string::npos) {
            return true;
        }
    }

    // Также проверить на структуру файла языка: {1,{0,{2,{1,0,guid},"язык",
    if (content.find("{1,") != std::string::npos &&
        content.find("{0,") != std::string::npos &&
        content.find("{2,") != std::string::npos &&
        content.find("\"язык\"") != std::string::npos) {
        return true;
    }

    return false;
}

std::string ParseMetadataCommand::extractRootFileGuid(const boost::filesystem::path& rootFilePath) {
    try {
        std::ifstream file(rootFilePath.string(), std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            return "";
        }

        // Парсим содержимое в формате {2,GUID,...}
        // Ищем GUID между первой и второй запятой
        size_t firstComma = content.find(',');
        if (firstComma == std::string::npos) {
            return "";
        }

        size_t secondComma = content.find(',', firstComma + 1);
        if (secondComma == std::string::npos) {
            return "";
        }

        // Извлекаем GUID после первой запятой
        std::string guid = content.substr(firstComma + 1, secondComma - firstComma - 1);

        // Удаляем пробелы и фигурные скобки
        guid.erase(std::remove_if(guid.begin(), guid.end(), [](char c) {
            return std::isspace(c) || c == '{' || c == '}';
        }), guid.end());

        return guid;

    } catch (const std::exception&) {
        return "";
    }
}

std::string ParseMetadataCommand::processRootConfigurationFile(const std::string& inputDir,
                                                              const std::string& outputDir) {
    try {
        // Ищем файл root в исходном каталоге
        fs::path rootFilePath = fs::path(inputDir) / "root";

        if (!fs::exists(rootFilePath)) {
            std::cerr << "Файл 'root' не найден в каталоге '" << inputDir << "'" << std::endl;
            return "";
        }

        if (!fs::is_regular_file(rootFilePath)) {
            std::cerr << "'root' не является файлом" << std::endl;
            return "";
        }

        // Извлекаем GUID из файла root
        std::string rootGuid = extractRootFileGuid(rootFilePath);
        if (rootGuid.empty()) {
            std::cerr << "Не удалось извлечь GUID из файла root" << std::endl;
            return "";
        }

        // Копируем файл root в каталог Конфигурация
        bool success = copyFileToCategory(rootFilePath, "Конфигурация", outputDir, inputDir);

        if (!success) {
            std::cout << "✗ Ошибка копирования файла root" << std::endl;
            return "";
        }

        categoryFiles_["Конфигурация"].push_back("root");
        std::cout << "✓ Корневой файл root → Конфигурация" << std::endl;

        // Ищем файл с GUID конфигурации в исходном каталоге
        fs::path configFilePath = fs::path(inputDir) / rootGuid;

        if (!fs::exists(configFilePath)) {
            std::cerr << "Корневой файл конфигурации '" << rootGuid << "' не найден в каталоге '" << inputDir << "'" << std::endl;
            return "";
        }

        if (!fs::is_regular_file(configFilePath)) {
            std::cerr << "'" << configFilePath.string() << "' не является файлом" << std::endl;
            return "";
        }

        // Копируем корневой файл конфигурации в каталог Конфигурация
        success = copyFileToCategory(configFilePath, "Конфигурация", outputDir, inputDir);

        if (success) {
            categoryFiles_["Конфигурация"].push_back(configFilePath.filename().string());
            std::cout << "✓ Корневой файл конфигурации: " << configFilePath.filename().string() << " → Конфигурация" << std::endl;
            return rootGuid;
        } else {
            std::cout << "✗ Ошибка копирования корневого файла конфигурации: " << configFilePath.filename().string() << std::endl;
            return "";
        }

    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Ошибка обработки корневого файла конфигурации: " << ex.what() << std::endl;
        return "";
    }
}

void ParseMetadataCommand::parseRootConfigForFileGuids(const boost::filesystem::path& rootConfigFilePath) {
    try {
        std::ifstream file(rootConfigFilePath.string(), std::ios::binary);
        if (!file.is_open()) {
            return;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            return;
        }

        // Try to parse using ConfigStructureParser first for better accuracy
        ConfigStructureParser configParser;
        if (configParser.loadFromString(content) && configParser.parse()) {
            // Use parsed catalogs
            const auto& catalogs = configParser.getCatalogs();
            for (const auto& catalog : catalogs) {
                std::string catalog_guid = catalog->guid;
                std::transform(catalog_guid.begin(), catalog_guid.end(), catalog_guid.begin(), ::tolower);
                fileGuidToCategoryMap_[catalog_guid] = std::string((const char*)md_Catalogs);
                std::cout << "Mapped catalog from ConfigStructureParser: " << catalog_guid << " → Справочники" << std::endl;
            }

            // Use parsed languages
            const auto& languages = configParser.getLanguages();
            for (const auto& lang : languages) {
                std::string lang_guid = lang->guid;
                std::transform(lang_guid.begin(), lang_guid.end(), lang_guid.begin(), ::tolower);
                fileGuidToCategoryMap_[lang_guid] = std::string((const char*)md_Languages);
                std::cout << "Mapped language from ConfigStructureParser: " << lang_guid << " → Языки" << std::endl;
            }
        } else {
            // Fallback to tree-based parsing
            std::cout << "ConfigStructureParser failed, falling back to tree parsing" << std::endl;
            tree* parsedTree = parse_1Ctext(content, rootConfigFilePath.string());
            if (!parsedTree) {
                return;
            }

            // Рекурсивно обходим дерево и ищем структуры {GUID_категории, ...}
            traverseTreeForCategoryFileGuids(parsedTree);

            // Освобождаем память
            delete parsedTree;
        }

    } catch (const std::exception&) {
        // Игнорируем ошибки парсинга
    }
}

void ParseMetadataCommand::traverseTreeForCategoryFileGuids(tree* node) {
    if (!node) return;

    // Ищем узлы типа списка (кортежи)
    if (node->get_type() == node_type::nd_list && node->get_num_subnode() >= 3) {
        tree* firstChild = node->get_subnode(0);
        if (firstChild && firstChild->get_type() == node_type::nd_guid) {
            std::string categoryGuid = firstChild->get_value().c_str();

            // Проверяем, является ли это GUID категории
            auto categoryIt = guidToCategoryMap_.find(categoryGuid);
            if (categoryIt != guidToCategoryMap_.end()) {
                std::string categoryName = categoryIt->second;

                // Извлекаем GUID'ы файлов начиная с третьего элемента (после count)
                for (int i = 2; i < node->get_num_subnode(); ++i) {
                    tree* guidNode = node->get_subnode(i);
                    if (guidNode && guidNode->get_type() == node_type::nd_guid) {
                        std::string fileGuid = guidNode->get_value().c_str();
                        if (!fileGuid.empty()) {
                            fileGuidToCategoryMap_[fileGuid] = categoryName;
                        }
                    }
                }
            }
        }
    }

    // Рекурсивно обходим дочерние узлы
    for (int i = 0; i < node->get_num_subnode(); ++i) {
        tree* child = node->get_subnode(i);
        traverseTreeForCategoryFileGuids(child);
    }
}

void ParseMetadataCommand::initializeCategoryMappings() {
    // Мапинг GUID'ов к именам категорий
    // Преобразуем GUID'ы в строки и категории в строки
    guidToCategoryMap_[std::string(GUID_Subsystems)] = std::string((const char*)md_Subsystems);
    guidToCategoryMap_[std::string(GUID_CommonModules)] = std::string((const char*)md_CommonModules);
    guidToCategoryMap_[std::string(GUID_SessionParameters)] = std::string((const char*)md_SessionParameters);
    guidToCategoryMap_[std::string(GUID_Roles)] = std::string((const char*)md_Roles);
    guidToCategoryMap_[std::string(GUID_CommonAttributes)] = std::string((const char*)md_CommonAttributes);
    guidToCategoryMap_[std::string(GUID_ExchangePlans)] = std::string((const char*)md_ExchangePlans);
    guidToCategoryMap_[std::string(GUID_FilterCriteria)] = std::string((const char*)md_FilterCriteria);
    guidToCategoryMap_[std::string(GUID_EventSubscriptions)] = std::string((const char*)md_EventSubscriptions);
    guidToCategoryMap_[std::string(GUID_ScheduledJobs)] = std::string((const char*)md_ScheduledJobs);
    guidToCategoryMap_[std::string(GUID_FunctionalOptions)] = std::string((const char*)md_FunctionalOptions);
    guidToCategoryMap_[std::string(GUID_FunctionalOptionsParameters)] = std::string((const char*)md_FunctionalOptionsParameters);
    guidToCategoryMap_[std::string(GUID_DefinedTypes)] = std::string((const char*)md_DefinedTypes);
    guidToCategoryMap_[std::string(GUID_SettingsStorages)] = std::string((const char*)md_SettingsStorages);
    guidToCategoryMap_[std::string(GUID_CommonForms)] = std::string((const char*)md_CommonForms);
    guidToCategoryMap_[std::string(GUID_CommonCommands)] = std::string((const char*)md_CommonCommands);
    guidToCategoryMap_[std::string(GUID_CommandGroups)] = std::string((const char*)md_CommandGroups);
    guidToCategoryMap_[std::string(GUID_Interfaces)] = std::string((const char*)md_Interfaces);
    guidToCategoryMap_[std::string(GUID_CommonTemplates)] = std::string((const char*)md_CommonTemplates);
    guidToCategoryMap_[std::string(GUID_CommonPictures)] = std::string((const char*)md_CommonPictures);
    guidToCategoryMap_[std::string(GUID_XDTOPackages)] = std::string((const char*)md_XDTOPackages);
    guidToCategoryMap_[std::string(GUID_WebServices)] = std::string((const char*)md_WebServices);
    guidToCategoryMap_[std::string(GUID_HTTPServices)] = std::string((const char*)md_HTTPServices);
    guidToCategoryMap_[std::string(GUID_WSReferences)] = std::string((const char*)md_WSReferences);
    guidToCategoryMap_[std::string(GUID_StyleItems)] = std::string((const char*)md_StyleItems);
    guidToCategoryMap_[std::string(GUID_Styles)] = std::string((const char*)md_Styles);
    guidToCategoryMap_[std::string(GUID_Languages)] = std::string((const char*)md_Languages);

    // Конфигурационные объекты
    guidToCategoryMap_[std::string(GUID_Catalogs)] = std::string((const char*)md_Catalogs);
    guidToCategoryMap_[std::string(GUID_Constants)] = std::string((const char*)md_Constants);
    guidToCategoryMap_[std::string(GUID_Documents)] = std::string((const char*)md_Documents);
    guidToCategoryMap_[std::string(GUID_Numerators)] = std::string((const char*)md_DocumentNumerators);
    guidToCategoryMap_[std::string(GUID_JournDocuments)] = std::string((const char*)md_DocumentJournals);
    guidToCategoryMap_[std::string(GUID_Enums)] = std::string((const char*)md_Enums);
    guidToCategoryMap_[std::string(GUID_Reports)] = std::string((const char*)md_Reports);
    guidToCategoryMap_[std::string(GUID_DataProcessors)] = std::string((const char*)md_DataProcessors);
    guidToCategoryMap_[std::string(GUID_ChartOfCharacteristicTypes)] = std::string((const char*)md_ChartsOfCharacteristicTypes);
    guidToCategoryMap_[std::string(GUID_ChartsOfAccounts)] = std::string((const char*)md_ChartOfAccounts);
    guidToCategoryMap_[std::string(GUID_ChartsOfCalculationTypes)] = std::string((const char*)md_ChartOfCalculationTypes);
    guidToCategoryMap_[std::string(GUID_InformationRegisters)] = std::string((const char*)md_InformationRegisters);
    guidToCategoryMap_[std::string(GUID_AccumulationRegisters)] = std::string((const char*)md_AccumulationRegisters);
    guidToCategoryMap_[std::string(GUID_AccountingRegisters)] = std::string((const char*)md_AccountingRegisters);
    guidToCategoryMap_[std::string(GUID_CalculationRegisters)] = std::string((const char*)md_CalculationRegisters);
    guidToCategoryMap_[std::string(GUID_BusinessProcesses)] = std::string((const char*)md_BusinessProcesses);
    guidToCategoryMap_[std::string(GUID_Tasks)] = std::string((const char*)md_Tasks);
    guidToCategoryMap_[std::string(GUID_ExternalDataSources)] = std::string((const char*)md_ExternalDataSources);
}

} // namespace v8unpack
