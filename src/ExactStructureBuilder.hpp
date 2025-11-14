#ifndef EXACT_STRUCTURE_BUILDER_HPP
#define EXACT_STRUCTURE_BUILDER_HPP

#include <boost/property_tree/ptree.hpp>
#include <string>

namespace pt = boost::property_tree;

class ExactStructureBuilder {
public:
    // Строим точное соответствие {{216,0,{80325,0}}}
    static pt::ptree buildExactStructure();
    
    // Парсинг строки в структуру PropertyTree
    static pt::ptree parseFromString(const std::string& input);
    
    // Функция для вывода структуры в читаемом формате
    static void printStructure(const pt::ptree& tree, const std::string& name = "Structure");
    
    // Функция для проверки соответствия исходной структуре
    static bool verifyStructure(const pt::ptree& tree);
    
    // Функция для сериализации в строку (имитация оригинального формата)
    static std::string serializeToOriginalFormat(const pt::ptree& tree);

private:
    // Вспомогательные функции для парсинга
    static pt::ptree parseStructure(const std::string& input, size_t& pos);
    static int parseNumber(const std::string& input, size_t& pos);
    static void skipWhitespace(const std::string& input, size_t& pos);
    
    // Рекурсивный вывод дерева
    static void printTree(const pt::ptree& tree, int indent);
    
    // Рекурсивная сериализация в оригинальном формате
    static void serializeTree(std::stringstream& ss, const pt::ptree& tree, int level);
};

#endif // EXACT_STRUCTURE_BUILDER_HPP