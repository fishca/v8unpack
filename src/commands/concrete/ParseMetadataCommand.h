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
     * Извлечь GUID из файла root
     */
    std::string extractRootFileGuid(const boost::filesystem::path& rootFilePath);

    /**
     * Обработать корневой файл конфигурации
     */
    bool processRootConfigurationFile(const std::string& inputDir,
                                     const std::string& outputDir,
                                     const std::string& rootGuid);

private:
    std::shared_ptr<MetadataAnalyzer> metadataAnalyzer_;
    std::map<std::string, std::string> guidToCategoryMap_;

    // Статистика классификации
    std::map<std::string, std::vector<std::string>> categoryFiles_;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_PARSE_METADATA_COMMAND_H
