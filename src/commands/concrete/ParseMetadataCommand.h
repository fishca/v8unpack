#ifndef COMMANDS_CONCRETE_PARSE_METADATA_COMMAND_H
#define COMMANDS_CONCRETE_PARSE_METADATA_COMMAND_H

#include <boost/filesystem.hpp>
#include <map>
#include <vector>
#include <string>

#include "../Command.h"
#include "../../metadata/MetadataAnalyzer.h"
#include "../../parse_tree.h"  // для tree*

namespace v8unpack {

class ParseMetadataCommand : public Command {
public:
    explicit ParseMetadataCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~ParseMetadataCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;

private:
    /**
     * Сканировать каталог и собрать все файлы для анализа
     */
    std::vector<boost::filesystem::path> scanDirectory(const std::string& inputDir);

    /**
     * Проанализировать отдельный файл и классифицировать его
     */
    std::string classifyFile(const boost::filesystem::path& filePath);

    /**
     * Извлечь все GUID'ы из дерева parse_1Ctext
     */
    std::vector<std::string> extractGuidsFromTree(tree* root);

    /**
     * Рекурсивный обход дерева для поиска GUID'ов
     */
    void traverseTreeForGuids(tree* node, std::vector<std::string>& guids);

    /**
     * Преобразовать GUID в имя каталога
     */
    std::string guidToCategoryName(const std::string& guid);

    /**
     * Создать структуру каталогов в CFSRC
     */
    bool createDirectoryStructure(const std::string& outputDir);

    /**
     * Скопировать файл в соответствующий каталог
     */
    bool copyFileToCategory(const boost::filesystem::path& sourceFile,
                           const std::string& categoryName,
                           const std::string& outputDir,
                           const std::string& inputDir);

    /**
     * Получить имя категории для специальных файлов (root, version, versions)
     */
    std::string getSpecialFileCategory(const boost::filesystem::path& filePath);

    /**
     * Создать отчёт о классификации
     */
    void printClassificationReport(const std::map<std::string, std::vector<std::string>>& categoryStats);

    /**
     * Инициализировать маппинг GUID → имён категорий
     */
    void initializeCategoryMappings();

    /**
     * Извлечь GUID из имени файла (убрать расширения типа .0, .1c)
     */
    std::string extractGuidFromFilename(const std::string& filename);

    /**
     * Проверить, является ли содержимое файла описанием языка
     */
    bool isLanguageFile(const std::string& content);

    /**
     * Извлечь GUID из файла root
     */
    std::string extractRootFileGuid(const boost::filesystem::path& rootFilePath);

    /**
     * Обработать корневой файл конфигурации
     */
    std::string processRootConfigurationFile(const std::string& inputDir,
                                             const std::string& outputDir);

    /**
     * Парсит корневой файл конфигурации и извлекает GUID'ы файлов для каждой категории
     */
    void parseRootConfigForFileGuids(const boost::filesystem::path& rootConfigFilePath);

    /**
     * Рекурсивный обход дерева для поиска GUID'ов файлов в категориях
     */
    void traverseTreeForCategoryFileGuids(tree* node);

    /**
     * Найти категорию родительского справочника для команды
     */
    std::string findParentCatalogCategory(const boost::filesystem::path& filePath, const std::string& commandGuid);

    /**
     * Проверить, содержит ли справочник команду с указанным GUID
     */
    bool isCatalogContainsCommand(const std::string& catalogGuid, const std::string& commandGuid, const std::string& inputDir);

    /**
     * Получить имя справочника по его GUID
     */
    std::string getCatalogNameByGuid(const std::string& catalogGuid);

    /**
     * Извлечь команды из файлов справочников
     */
    void extractCommandsFromCatalogFiles(const std::string& inputDir);

    /**
     * Извлечь GUID'ы команд из секции команд справочников
     */
    void extractCommandGuidsFromCatalogCommandsSection(tree* commandsSectionNode);

    /**
     * Рекурсивный обход дерева для поиска GUID'ов команд
     */
    void traverseTreeForCommandGuids(tree* node);

    /**
     * Проверить, является ли файл командой справочника
     */
    bool isLikelyCatalogCommand(const boost::filesystem::path& filePath, const std::string& content);

private:
    std::shared_ptr<MetadataAnalyzer> metadataAnalyzer_;
    std::map<std::string, std::string> guidToCategoryMap_;

    // Мапа GUID файлов -> категория из корневого файла конфигурации
    std::map<std::string, std::string> fileGuidToCategoryMap_;

    // Мапа GUID справочников -> имена справочников
    std::map<std::string, std::string> catalogGuidToNameMap_;

    // Статистика классификации
    std::map<std::string, std::vector<std::string>> categoryFiles_;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_PARSE_METADATA_COMMAND_H
