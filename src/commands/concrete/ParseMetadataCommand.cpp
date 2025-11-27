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

        // Извлекаем команды из файлов справочников
        extractCommandsFromCatalogFiles(inputDir);

        std::cout << "DEBUG: fileGuidToCategoryMap_ contains:" << std::endl;
        for (const auto& pair : fileGuidToCategoryMap_) {
            std::cout << "  " << pair.first << " -> " << pair.second << std::endl;
        }

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
    std::cout << "DEBUG: Classifying file: " << filePath.string() << std::endl;

    // Сначала проверяем специальные файлы
    std::string specialCategory = getSpecialFileCategory(filePath);
    if (!specialCategory.empty()) {
        std::cout << "DEBUG: Special category: " << specialCategory << std::endl;
        return specialCategory;
    }

    // Извлекаем GUID из имени файла (убираем расширения типа .0, .1c и т.д.)
    std::string filename = filePath.filename().string();
    std::string guid = extractGuidFromFilename(filename);

    std::cout << "DEBUG: Initial guid from filename: " << guid << std::endl;

    // Для файлов в подкаталогах команд (типа GUID.2/text) используем GUID из имени каталога
    if (filePath.parent_path() != filePath.root_path() && filePath.parent_path().filename() != ".") {
        std::string parentDirName = filePath.parent_path().filename().string();
        std::string parentGuid = extractGuidFromFilename(parentDirName);
        if (!parentGuid.empty() && parentGuid != filename) {
            guid = parentGuid;
            std::cout << "DEBUG: Using parent directory GUID: " << guid << " for file: " << filename << std::endl;
        }
    }

    // Проверяем, есть ли GUID файла в мапе из корневого файла конфигурации
    auto fileGuidIt = fileGuidToCategoryMap_.find(guid);
    if (fileGuidIt != fileGuidToCategoryMap_.end() && fileGuidIt->second != std::string((const char*)md_Languages)) {
        std::cout << "DEBUG: Found in fileGuidToCategoryMap_: " << fileGuidIt->second << std::endl;
        return fileGuidIt->second;
    }

    std::cout << "DEBUG: Not found in fileGuidToCategoryMap_, checking if catalog command" << std::endl;

    // Читаем содержимое файла для проверки на команду справочника
    std::string content;
    try {
        std::ifstream file(filePath.string(), std::ios::binary);
        if (!file.is_open()) {
            std::cout << "DEBUG: Cannot open file for reading" << std::endl;
            return "";
        }

        content.assign((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            std::cout << "DEBUG: File content is empty" << std::endl;
            return "";
        }

        // Дополнительная логика: проверяем, может ли файл быть командой справочника
        // по структуре каталога или по содержимому
        std::cout << "DEBUG: Checking file: " << filePath.string() << ", guid: " << guid << std::endl;
        bool isLikelyCommand = isLikelyCatalogCommand(filePath, content);
        std::cout << "DEBUG: isLikelyCatalogCommand result: " << (isLikelyCommand ? "true" : "false") << std::endl;
        if (isLikelyCommand) {
            std::cout << "DEBUG: Detected likely catalog command: " << filePath.filename().string() << std::endl;
            // Для команд справочников сразу ищем родительский справочник
            std::string parentCategory = findParentCatalogCategory(filePath, guid);
            if (!parentCategory.empty()) {
                std::cout << "DEBUG: Found parent category: " << parentCategory << " for command " << filePath.filename().string() << std::endl;
                return parentCategory + "/Команды";
            }
            std::cout << "DEBUG: No parent category found for command " << filePath.filename().string() << ", using default" << std::endl;
            return "Справочники/Команды";
        }

        std::cout << "DEBUG: Not a catalog command, proceeding to parsing" << std::endl;
    } catch (const std::exception&) {
        std::cout << "DEBUG: Exception reading file content" << std::endl;
        return "";
    }

    // Попробуем распарсить файл для извлечения GUID'ов
    try {
        // Парсим как скобочную структуру 1C
        tree* parsedTree = parse_1Ctext(content, filePath.string());
        if (parsedTree) {
            // Извлекаем GUID'ы из дерева
            std::vector<std::string> guids = extractGuidsFromTree(parsedTree);

            // Освобождаем память
            delete parsedTree;

            // Ищем основной GUID файла (обычно первый в списке)
            for (const auto& guid : guids) {
                std::string category = guidToCategoryName(guid);
                if (!category.empty()) {
                    // Если это команда справочника, ищем, к какому именно справочнику она принадлежит
                    if (category == "Команды справочников") {
                        std::string parentCategory = findParentCatalogCategory(filePath, guid);
                        if (!parentCategory.empty()) {
                            return parentCategory + "/Команды";
                        }
                    }
                    return category;
                }
            }
        }

    } catch (const std::exception&) {
        // Игнорируем ошибки парсинга отдельных файлов
    }

    // Если не удалось распарсить или не найдены GUID'ы, проверить на язык по содержимому
    if (isLanguageFile(content)) {
        std::cout << "DEBUG: Detected language file" << std::endl;
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
        "\"हिन्दी\"", "\"বাংলা\"", "\"தமிழ்\"", "\"ардо\""
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
            catalogGuidToNameMap_[catalog_guid] = catalog->name;
            std::cout << "Mapped catalog from ConfigStructureParser: " << catalog_guid << " → Справочники (" << catalog->name << ")" << std::endl;
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

            // Специальная обработка для секции команд справочников
            if (categoryGuid == "4fe87c89-9ad4-43f6-9fdb-9dc83b3879c6") {
                // Это секция команд справочников
                extractCommandGuidsFromCatalogCommandsSection(node);
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
    
    // Специальные GUID'ы для команд внутри объектов метаданных
    guidToCategoryMap_[std::string(GUID_RefCommands)] = "Команды справочников";
    guidToCategoryMap_[std::string(GUID_SelCritCommands)] = "Команды критериев отбора";
    guidToCategoryMap_[std::string(GUID_TasksCommands)] = "Команды задач";
    guidToCategoryMap_[std::string(GUID_BPCommands)] = "Команды бизнес-процессов";
    guidToCategoryMap_[std::string(GUID_ProcessingCommand)] = "Команды обработок";
    guidToCategoryMap_[std::string(GUID_InfoRegCommand)] = "Команды регистров сведений";
    guidToCategoryMap_[std::string(GUID_AccRegCommand)] = "Команды регистров бухгалтерии";
    guidToCategoryMap_[std::string(GUID_DocCommand)] = "Команды документов";
    guidToCategoryMap_[std::string(GUID_ExchPlanCommand)] = "Команды планов обмена";
    guidToCategoryMap_[std::string(GUID_DocJrnlCommand)] = "Команды журналов документов";
    guidToCategoryMap_[std::string(GUID_AccPlnCommand)] = "Команды планов счетов";
    guidToCategoryMap_[std::string(GUID_RegAcmCommand)] = "Команды регистров накопления";
    guidToCategoryMap_[std::string(GUID_BsnPrcCommand)] = "Команды бизнес-процессов";
}

std::string ParseMetadataCommand::findParentCatalogCategory(const fs::path& filePath, const std::string& commandGuid) {
    // Для определения принадлежности команды к конкретному справочнику,
    // анализируем структуру файлов справочников в исходном каталоге

    std::string inputDir = filePath.parent_path().parent_path().string();

    // Перебираем все GUID'ы справочников из мапы catalogGuidToNameMap_
    for (const auto& pair : catalogGuidToNameMap_) {
        const std::string& catalogGuid = pair.first;
        const std::string& catalogName = pair.second;

        // Проверяем, содержит ли справочник данную команду
        if (isCatalogContainsCommand(catalogGuid, commandGuid, inputDir)) {
            std::cout << "DEBUG: Command " << commandGuid << " belongs to catalog " << catalogName << " (" << catalogGuid << ")" << std::endl;
            return "Справочники/" + catalogName;
        }
    }

    // Если не нашли принадлежность, возвращаем общую категорию
    std::cout << "DEBUG: Command " << commandGuid << " not found in any catalog, using default category" << std::endl;
    return "Справочники";
}

bool ParseMetadataCommand::isCatalogContainsCommand(const std::string& catalogGuid, const std::string& commandGuid, const std::string& inputDir) {
    try {
        // Ищем файл структуры справочника
        fs::path catalogFilePath = fs::path(inputDir) / catalogGuid;

        if (!fs::exists(catalogFilePath) || !fs::is_regular_file(catalogFilePath)) {
            std::cout << "DEBUG: Catalog file not found: " << catalogFilePath.string() << std::endl;
            return false;
        }

        // Читаем содержимое файла структуры справочника
        std::ifstream file(catalogFilePath.string(), std::ios::binary);
        if (!file.is_open()) {
            std::cout << "DEBUG: Cannot open catalog file: " << catalogFilePath.string() << std::endl;
            return false;
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            std::cout << "DEBUG: Catalog file is empty: " << catalogFilePath.string() << std::endl;
            return false;
        }

        std::cout << "DEBUG: Checking catalog " << catalogGuid << " for command " << commandGuid << std::endl;

        // Ищем секцию команд с GUID 4fe87c89-9ad4-43f6-9fdb-9dc83b3879c6
        std::string commandSectionGuid = "4fe87c89-9ad4-43f6-9fdb-9dc83b3879c6";
        size_t commandSectionPos = content.find(commandSectionGuid);
        if (commandSectionPos == std::string::npos) {
            std::cout << "DEBUG: Command section not found in catalog " << catalogGuid << std::endl;
            return false;
        }

        std::cout << "DEBUG: Command section found at position " << commandSectionPos << " in catalog " << catalogGuid << std::endl;

        // Ищем команду с заданным GUID в секции команд
        size_t commandPos = content.find(commandGuid, commandSectionPos);
        if (commandPos == std::string::npos) {
            std::cout << "DEBUG: Command " << commandGuid << " not found in catalog " << catalogGuid << std::endl;
            return false;
        }

        std::cout << "DEBUG: Command " << commandGuid << " found at position " << commandPos << " in catalog " << catalogGuid << std::endl;

        // Проверяем, что команда находится внутри секции команд
        return commandPos > commandSectionPos;

    } catch (const std::exception& ex) {
        std::cout << "DEBUG: Exception in isCatalogContainsCommand: " << ex.what() << std::endl;
        return false;
    }
}

void ParseMetadataCommand::extractCommandGuidsFromCatalogCommandsSection(tree* commandsSectionNode) {
    if (!commandsSectionNode || commandsSectionNode->get_type() != node_type::nd_list) {
        return;
    }

    // Рекурсивно обходим секцию команд и ищем GUID'ы команд
    traverseTreeForCommandGuids(commandsSectionNode);
}

void ParseMetadataCommand::traverseTreeForCommandGuids(tree* node) {
    if (!node) return;

    // Ищем GUID'ы команд (они находятся в структурах типа {2, guid_command, ...})
    if (node->get_type() == node_type::nd_list && node->get_num_subnode() >= 2) {
        tree* firstChild = node->get_subnode(0);
        if (firstChild && firstChild->get_type() == node_type::nd_number) {
            std::string numberValue = firstChild->get_value().c_str();
            if (numberValue == "2") {  // Структура команды начинается с числа 2
                tree* guidNode = node->get_subnode(1);
                if (guidNode && guidNode->get_type() == node_type::nd_guid) {
                    std::string commandGuid = guidNode->get_value().c_str();
                    if (!commandGuid.empty()) {
                        std::transform(commandGuid.begin(), commandGuid.end(), commandGuid.begin(), ::tolower);
                        fileGuidToCategoryMap_[commandGuid] = "Команды справочников";
                        std::cout << "DEBUG: Found catalog command GUID: " << commandGuid << std::endl;
                    }
                }
            }
        }
    }

    // Рекурсивно обходим дочерние узлы
    for (int i = 0; i < node->get_num_subnode(); ++i) {
        tree* child = node->get_subnode(i);
        traverseTreeForCommandGuids(child);
    }
}

void ParseMetadataCommand::extractCommandsFromCatalogFiles(const std::string& inputDir) {
    std::cout << "DEBUG: Extracting commands from catalog files..." << std::endl;

    // Проходим по всем справочникам из catalogGuidToNameMap_
    for (const auto& pair : catalogGuidToNameMap_) {
        const std::string& catalogGuid = pair.first;
        const std::string& catalogName = pair.second;

        // Ищем файл справочника
        fs::path catalogFilePath = fs::path(inputDir) / catalogGuid;

        if (!fs::exists(catalogFilePath) || !fs::is_regular_file(catalogFilePath)) {
            std::cout << "DEBUG: Catalog file not found: " << catalogFilePath.string() << std::endl;
            continue;
        }

        // Парсим файл справочника и извлекаем команды
        try {
            std::ifstream file(catalogFilePath.string(), std::ios::binary);
            if (!file.is_open()) {
                std::cout << "DEBUG: Cannot open catalog file: " << catalogFilePath.string() << std::endl;
                continue;
            }

            std::string content((std::istreambuf_iterator<char>(file)),
                               std::istreambuf_iterator<char>());
            file.close();

            if (content.empty()) {
                continue;
            }

            // Парсим файл справочника
            tree* catalogTree = parse_1Ctext(content, catalogFilePath.string());
            if (catalogTree) {
                // Ищем секцию команд и извлекаем GUID'ы команд
                traverseTreeForCategoryFileGuids(catalogTree);
                delete catalogTree;
            } else {
                std::cout << "DEBUG: Failed to parse catalog file: " << catalogFilePath.string() << std::endl;
            }

        } catch (const std::exception& ex) {
            std::cout << "DEBUG: Error parsing catalog file " << catalogFilePath.string() << ": " << ex.what() << std::endl;
        }
    }

    std::cout << "DEBUG: Finished extracting commands from catalog files" << std::endl;
}

std::string ParseMetadataCommand::getCatalogNameByGuid(const std::string& catalogGuid) {
    // Простая реализация - возвращаем общий путь "Справочники"
    return "Справочники";
}

bool ParseMetadataCommand::isLikelyCatalogCommand(const boost::filesystem::path& filePath, const std::string& content) {
    // Проверяем, является ли файл командой справочника по расположению в подкаталоге
    // с именем GUID команды и по содержимому (для файлов text/module)

    std::cout << "DEBUG: Checking if file is catalog command: " << filePath.string() << std::endl;

    // Проверяем, находится ли файл в подкаталоге (команда справочника)
    if (filePath.parent_path() != filePath.root_path() && filePath.parent_path().filename() != ".") {
        std::string parentDirName = filePath.parent_path().filename().string();
        std::string filename = filePath.filename().string();

        std::cout << "DEBUG: Parent dir: " << parentDirName << ", filename: " << filename << std::endl;

        // Проверяем, является ли имя родительского каталога GUID'ом с расширением вида .2
        size_t dotPos = parentDirName.find('.');
        if (dotPos != std::string::npos) {
            std::string guidPart = parentDirName.substr(0, dotPos);
            std::string extPart = parentDirName.substr(dotPos + 1);

            std::cout << "DEBUG: guidPart: " << guidPart << ", extPart: " << extPart << std::endl;

            // Проверяем, что GUID часть выглядит как настоящий GUID (36 символов с дефисами на нужных местах)
            if (guidPart.length() == 36 &&
                guidPart[8] == '-' && guidPart[13] == '-' && guidPart[18] == '-' && guidPart[23] == '-' &&
                (extPart == "0" || extPart == "1" || extPart == "2" || extPart == "3" || extPart == "4" || extPart == "5")) {

                std::cout << "DEBUG: GUID format is valid" << std::endl;

                // Если это основной файл команды (text или module), проверяем содержимое
                if (filename == "text" || filename == "module") {
                    bool hasCommandHandler = content.find("ОбработкаКоманды") != std::string::npos ||
                        content.find("Procedure") != std::string::npos ||
                        content.find("Процедура") != std::string::npos ||
                        content.find("&НаКлиенте") != std::string::npos;
                    std::cout << "DEBUG: Has command handler: " << (hasCommandHandler ? "true" : "false") << std::endl;
                    if (hasCommandHandler) {
                        return true;
                    }
                } else {
                    // Для других файлов в том же каталоге (info, form и т.д.) - тоже считаем командами
                    std::cout << "DEBUG: Non-text file in command directory, treating as command" << std::endl;
                    return true;
                }
            } else {
                std::cout << "DEBUG: GUID format is invalid" << std::endl;
            }
        } else {
            std::cout << "DEBUG: No dot in parent dir name" << std::endl;
        }
    } else {
        std::cout << "DEBUG: File not in subdirectory" << std::endl;
    }

    std::cout << "DEBUG: File is not a catalog command" << std::endl;
    return false;
}

} // namespace v8unpack
