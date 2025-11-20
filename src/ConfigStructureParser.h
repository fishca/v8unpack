#ifndef CONFIG_STRUCTURE_PARSER_H
#define CONFIG_STRUCTURE_PARSER_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_map>

// Forward declaration for tree class
struct tree;

namespace v8unpack {

// Структура для представления метаданного объекта
struct MetadataObject {
    std::string guid;
    std::string name;
    std::string type;
    std::string description;

    MetadataObject(const std::string& g, const std::string& n, const std::string& t, const std::string& d = "")
        : guid(g), name(n), type(t), description(d) {}

    friend std::ostream& operator<<(std::ostream& os, const MetadataObject& obj) {
        os << "GUID: " << obj.guid << std::endl;
        os << "Name: " << obj.name << std::endl;
        os << "Type: " << obj.type << std::endl;
        if (!obj.description.empty()) {
            os << "Description: " << obj.description << std::endl;
        }
        return os;
    }
};

// Структура для представления справочника (Каталог в 1С)
struct Catalog : public MetadataObject {
    bool hierarchical;
    bool folder;
    std::vector<std::string> attributes;

    Catalog(const std::string& g, const std::string& n, bool h = false, bool f = false)
        : MetadataObject(g, n, "Справочник"), hierarchical(h), folder(f) {}
};

// Структура для представления языка
struct Language : public MetadataObject {
    std::string code;
    bool default_language;

    Language(const std::string& g, const std::string& n, const std::string& c = "", bool def = false)
        : MetadataObject(g, n, "Язык"), code(c), default_language(def) {}
};

// Класс для парсинга структуры конфигурации 1C
class ConfigStructureParser {
private:
    std::string config_content_;
    std::vector<std::shared_ptr<MetadataObject>> catalogs_;
    std::vector<std::shared_ptr<MetadataObject>> languages_;
    std::unordered_map<std::string, int> found_types_count_;

    // Методы парсинга
    bool parseConfigStructure();
    bool parseMetadataTree(tree* root_tree);
    bool extractCatalogsAndLanguages(tree* metadata_tree);
    bool extractObjectsFromMetadataType(tree* type_tree, const std::string& type_name);
    void searchGuidsInText();

public:
    ConfigStructureParser();
    ~ConfigStructureParser();

    // Загрузка файла конфигурации
    bool loadFromFile(const std::string& file_path);
    bool loadFromString(const std::string& content);

    // Парсинг конфигурации
    bool parse();

    // Получение результатов
    const std::vector<std::shared_ptr<MetadataObject>>& getCatalogs() const {
        return catalogs_;
    }

    const std::vector<std::shared_ptr<MetadataObject>>& getLanguages() const {
        return languages_;
    }

    size_t getCatalogCount() const {
        return catalogs_.size();
    }

    size_t getLanguageCount() const {
        return languages_.size();
    }

    // Вывод информации
    void printCatalogs(std::ostream& os = std::cout) const;
    void printLanguages(std::ostream& os = std::cout) const;
    void printSummary(std::ostream& os = std::cout) const;
};

} // namespace v8unpack

#endif // CONFIG_STRUCTURE_PARSER_H
