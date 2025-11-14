#include "ExactStructureBuilder.hpp"
#include <boost/property_tree/info_parser.hpp>
#include <iostream>
#include <sstream>
#include <cctype>
#include <stdexcept>

// Строим точное соответствие {{216,0,{80325,0}}}
pt::ptree ExactStructureBuilder::buildExactStructure() {
    // Внешняя структура, содержащая одну внутреннюю структуру
    pt::ptree outer_struct;
    
    // Внутренняя структура с тремя элементами: 216, 0, {80325,0}
    pt::ptree inner_struct;
    
    // Элемент 1: 216
    pt::ptree elem1;
    elem1.put_value(216);
    inner_struct.push_back(std::make_pair("", elem1));
    
    // Элемент 2: 0
    pt::ptree elem2;
    elem2.put_value(0);
    inner_struct.push_back(std::make_pair("", elem2));
    
    // Элемент 3: вложенная структура {80325,0}
    pt::ptree elem3;
    pt::ptree nested_struct;
    
    // Вложенный элемент 1: 80325
    pt::ptree nested_elem1;
    nested_elem1.put_value(80325);
    nested_struct.push_back(std::make_pair("", nested_elem1));
    
    // Вложенный элемент 2: 0
    pt::ptree nested_elem2;
    nested_elem2.put_value(0);
    nested_struct.push_back(std::make_pair("", nested_elem2));
    
    elem3.add_child("", nested_struct);
    inner_struct.push_back(std::make_pair("", elem3));
    
    // Добавляем внутреннюю структуру во внешнюю
    outer_struct.add_child("", inner_struct);
    
    return outer_struct;
}

// Пропускаем пробельные символы
void ExactStructureBuilder::skipWhitespace(const std::string& input, size_t& pos) {
    while (pos < input.size() && std::isspace(input[pos])) {
        pos++;
    }
}

// Парсим число из строки
int ExactStructureBuilder::parseNumber(const std::string& input, size_t& pos) {
    skipWhitespace(input, pos);
    
    if (pos >= input.size() || !std::isdigit(input[pos])) {
        throw std::runtime_error("Ожидается число в позиции " + std::to_string(pos));
    }
    
    int result = 0;
    while (pos < input.size() && std::isdigit(input[pos])) {
        result = result * 10 + (input[pos] - '0');
        pos++;
    }
    
    return result;
}

// Рекурсивный парсинг структуры
pt::ptree ExactStructureBuilder::parseStructure(const std::string& input, size_t& pos) {
    pt::ptree result;
    
    skipWhitespace(input, pos);
    
    if (pos >= input.size() || input[pos] != '{') {
        throw std::runtime_error("Ожидается '{' в позиции " + std::to_string(pos));
    }
    
    pos++; // Пропускаем '{'
    
    bool first_element = true;
    
    while (pos < input.size()) {
        skipWhitespace(input, pos);
        
        if (input[pos] == '}') {
            pos++; // Пропускаем '}'
            break;
        }
        
        if (!first_element) {
            if (input[pos] == ',') {
                pos++; // Пропускаем запятую
                skipWhitespace(input, pos);
            } else {
                throw std::runtime_error("Ожидается ',' или '}' в позиции " + std::to_string(pos));
            }
        }
        
        if (input[pos] == '{') {
            // Вложенная структура
            pt::ptree nested = parseStructure(input, pos);
            pt::ptree wrapper;
            wrapper.add_child("", nested);
            result.push_back(std::make_pair("", wrapper));
        } else if (std::isdigit(input[pos])) {
            // Число
            int number = parseNumber(input, pos);
            pt::ptree number_node;
            number_node.put_value(number);
            result.push_back(std::make_pair("", number_node));
        } else {
            throw std::runtime_error("Неожиданный символ в позиции " + std::to_string(pos) + ": " + input[pos]);
        }
        
        first_element = false;
    }
    
    return result;
}

// Парсинг строки в структуру PropertyTree
pt::ptree ExactStructureBuilder::parseFromString(const std::string& input) {
    size_t pos = 0;
    pt::ptree result = parseStructure(input, pos);
    
    // Проверяем, что вся строка обработана
    skipWhitespace(input, pos);
    if (pos < input.size()) {
        throw std::runtime_error("Необработанные символы в конце строки: " + input.substr(pos));
    }
    
    return result;
}

// Функция для вывода структуры в читаемом формате
void ExactStructureBuilder::printStructure(const pt::ptree& tree, const std::string& name) {
    std::cout << "=== " << name << " ===" << std::endl;
    printTree(tree, 0);
}

// Функция для проверки соответствия исходной структуре
bool ExactStructureBuilder::verifyStructure(const pt::ptree& tree) {
    try {
        // Проверяем, что есть внешняя структура с одним элементом
        if (tree.size() != 1) return false;
        
        // Получаем внутреннюю структуру
        const pt::ptree& inner_struct = tree.begin()->second;
        if (inner_struct.size() != 3) return false;
        
        auto it = inner_struct.begin();
        
        // Проверяем первый элемент: 216
        if (it->second.get_value<int>() != 216) return false;
        ++it;
        
        // Проверяем второй элемент: 0
        if (it->second.get_value<int>() != 0) return false;
        ++it;
        
        // Проверяем третий элемент: вложенная структура {80325,0}
        const pt::ptree& nested_wrapper = it->second;
        if (nested_wrapper.size() != 1) return false;
        
        const pt::ptree& nested_struct = nested_wrapper.begin()->second;
        if (nested_struct.size() != 2) return false;
        
        auto nested_it = nested_struct.begin();
        if (nested_it->second.get_value<int>() != 80325) return false;
        ++nested_it;
        if (nested_it->second.get_value<int>() != 0) return false;
        
        return true;
        
    } catch (...) {
        return false;
    }
}

// Функция для сериализации в строку (имитация оригинального формата)
std::string ExactStructureBuilder::serializeToOriginalFormat(const pt::ptree& tree) {
    std::stringstream ss;
    serializeTree(ss, tree, 0);
    return ss.str();
}

// Рекурсивный вывод дерева
void ExactStructureBuilder::printTree(const pt::ptree& tree, int indent) {
    std::string indent_str(indent * 2, ' ');
    
    if (tree.empty()) {
        // Листовой узел
        std::cout << indent_str << tree.get_value<int>() << std::endl;
    } else {
        // Структура
        std::cout << indent_str << "{" << std::endl;
        for (const auto& child : tree) {
            printTree(child.second, indent + 1);
        }
        std::cout << indent_str << "}" << std::endl;
    }
}

// Рекурсивная сериализация в оригинальном формате
void ExactStructureBuilder::serializeTree(std::stringstream& ss, const pt::ptree& tree, int level) {
    if (tree.empty()) {
        // Листовой узел - просто значение
        ss << tree.get_value<int>();
    } else {
        // Структура - открываем фигурную скобку
        ss << "{";
        
        bool first = true;
        for (const auto& child : tree) {
            if (!first) {
                ss << ",";
            }
            first = false;
            
            serializeTree(ss, child.second, level + 1);
        }
        
        // Закрываем фигурную скобку
        ss << "}";
    }
}