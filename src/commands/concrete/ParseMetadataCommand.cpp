#include "ParseMetadataCommand.h"

#include "../../parse_tree.h"
#include "../../common.h"
#include "../../guids.h"
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>
#include <algorithm>
#include <regex>

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

        // Создаём структуру каталогов в CFSRC
        if (!createDirectoryStructure(outputDir)) {
            std::cerr << "Не удалось создать структуру каталогов в '" << outputDir << "'" << std::endl;
            return 1;
        }

        // Шаг 1: Ищем и обрабатываем файл root
        fs::path rootFilePath;
        bool rootFileFound = false;

        for (const auto& filePath : files) {
            if (filePath.filename().string() == "root") {
                rootFilePath = filePath;
                rootFileFound = true;
                break;
            }
        }

        if (!rootFileFound) {
            std::cerr << "Файл 'root' не найден в каталоге '" << inputDir << "'" << std::endl;
            return 1;
        }

        // Извлекаем GUID из файла root
        std::string rootGuid = extractRootFileGuid(rootFilePath);
        if (rootGuid.empty()) {
            std::cerr << "Не удалось извлечь GUID из файла root" << std::endl;
            return 1;
        }

        std::cout << "Найден GUID корневого файла конфигурации: " << rootGuid << std::endl;

        // Обрабатываем корневой файл конфигурации
        if (!processRootConfigurationFile(inputDir, outputDir, rootGuid)) {
            std::cerr << "Ошибка обработки корневого файла конфигурации" << std::endl;
            return 1;
        }

        // Шаг 2: Обрабатываем остальные файлы
        std::cout << "Обрабатываем остальные файлы..." << std::endl;

        for (const auto& filePath : files) {
            std::string filename = filePath.filename().string();

            // Пропускаем файл root и корневой файл конфигурации (они уже обработаны)
            if (filename == "root" || filename == rootGuid) {
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

    // Читаем и парсим содержимое файла
    try {
        std::ifstream file(filePath.string(), std::ios::binary);
        if (!file.is_open()) {
            return "";
        }

        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        file.close();

        if (content.empty()) {
            return "";
        }

        // Парсим как скобочную структуру 1C
        tree* parsedTree = parse_1Ctext(content, filePath.string());
        if (!parsedTree) {
            return "";
        }

        // Извлекаем GUID'ы из дерева
        std::vector<std::string> guids = extractGuidsFromTree(parsedTree);

        // Освобождаем память
        delete parsedTree;

        if (guids.empty()) {
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

        // Удаляем пробелы
        guid.erase(std::remove_if(guid.begin(), guid.end(), ::isspace), guid.end());

        return guid;

    } catch (const std::exception&) {
        return "";
    }
}

bool ParseMetadataCommand::processRootConfigurationFile(const std::string& inputDir,
                                                       const std::string& outputDir,
                                                       const std::string& rootGuid) {
    try {
        // Ищем файл с именем rootGuid в исходном каталоге
        fs::path configFilePath = fs::path(inputDir) / rootGuid;

        if (!fs::exists(configFilePath)) {
            std::cerr << "Корневой файл конфигурации '" << rootGuid << "' не найден в каталоге '" << inputDir << "'" << std::endl;
            return false;
        }

        if (!fs::is_regular_file(configFilePath)) {
            std::cerr << "'" << configFilePath.string() << "' не является файлом" << std::endl;
            return false;
        }

        // Копируем файл в каталог Конфигурация
        bool success = copyFileToCategory(configFilePath, "Конфигурация", outputDir, inputDir);

        if (success) {
            categoryFiles_["Конфигурация"].push_back(configFilePath.filename().string());
            std::cout << "✓ Корневой файл конфигурации: " << configFilePath.filename().string() << " → Конфигурация" << std::endl;
        } else {
            std::cout << "✗ Ошибка копирования корневого файла конфигурации: " << configFilePath.filename().string() << std::endl;
            return false;
        }

        return true;

    } catch (const fs::filesystem_error& ex) {
        std::cerr << "Ошибка обработки корневого файла конфигурации: " << ex.what() << std::endl;
        return false;
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
