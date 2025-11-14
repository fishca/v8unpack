#include "onesdata.hpp"
#include <stdexcept>
#include <sstream>
#include <cctype>

#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>
#include <boost/property_tree/json_parser.hpp>


// Реализация методов DataElement
DataElement::DataElement(std::string k, ValueVariant v) 
    : key(std::move(k)), value(std::move(v)) {}

// Реализация методов ObjectReference
ObjectReference::ObjectReference() : version(0), type(0) {}

ObjectReference::ObjectReference(UUID id, IntValue ver, IntValue t) 
    : uuid(id), version(ver), type(t) {}

// Реализация методов OneSDataStorage
OneSDataStorage::OneSDataStorage() = default;
OneSDataStorage::~OneSDataStorage() = default;

bool OneSDataStorage::loadFromString(const std::string& data) {
    try {
        // Очищаем предыдущие данные
        uuid_index.clear();
        text_index.clear();
        number_index.clear();
        data_pool.clear();
        
        ParserState state;
        state.data = data;
        state.position = 0;
        
        root_element = parseElement(state);
        buildIndexes(&root_element);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Load error: " << e.what() << std::endl;
        return false;
    }
}

std::vector<DataElement*> OneSDataStorage::findByUUID(const UUID& uuid) {
    auto it = uuid_index.find(uuid);
    if (it != uuid_index.end()) {
        return it->second;
    }
    return {};
}

std::vector<DataElement*> OneSDataStorage::findByText(const std::string& text) {
    auto it = text_index.find(text);
    if (it != text_index.end()) {
        return it->second;
    }
    return {};
}

std::vector<DataElement*> OneSDataStorage::findByNumber(IntValue number) {
    auto it = number_index.find(number);
    if (it != number_index.end()) {
        return it->second;
    }
    return {};
}

ptree OneSDataStorage::toPropertyTree() const {
    ptree result;
    serializeElement(root_element, result);
    return result;
}

void OneSDataStorage::exportToJson(const std::string& filename) {
    try {
        ptree tree = toPropertyTree();
        write_json(filename, tree);
        std::cout << "Data successfully exported to: " << filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Export error: " << e.what() << std::endl;
    }
}

size_t OneSDataStorage::getTotalElements() const {
    size_t count = 0;
    std::function<void(const DataElement&)> counter = [&](const DataElement& elem) {
        count++;
        for (const auto& child : elem.children) {
            counter(child);
        }
    };
    counter(root_element);
    return count;
}

// Приватные методы парсинга
DataElement OneSDataStorage::parseElement(ParserState& state) {
    skipWhitespace(state);
    
    if (state.position >= state.data.size()) {
        throw std::runtime_error("Unexpected end of data");
    }

    char current = state.data[state.position];
    
    if (current == '{') {
        return parseObjectOrArray(state);
    } else if (current == '"') {
        return parseString(state);
    } else if (isDigit(current) || current == '-') {
        return parseNumber(state);
    } else if (isAlpha(current) || current == '#') {
        return parseIdentifier(state);
    } else {
        throw std::runtime_error(std::string("Unexpected character: ") + current);
    }
}

DataElement OneSDataStorage::parseObjectOrArray(ParserState& state) {
    consumeChar(state, '{');
    DataElement element;
    element.is_array = true;

    skipWhitespace(state);
    
    while (state.position < state.data.size() && state.data[state.position] != '}') {
        skipWhitespace(state);
        
        // Проверяем, есть ли ключ
        if (isPotentialKey(state)) {
            auto key = parseKey(state);
            consumeChar(state, ':');
            auto value = parseElement(state);
            value.key = key;
            element.children.push_back(value);
        } else {
            element.children.push_back(parseElement(state));
        }
        
        skipWhitespace(state);
        
        if (state.data[state.position] == ',') {
            consumeChar(state, ',');
        }
    }
    
    consumeChar(state, '}');
    return element;
}

std::string OneSDataStorage::parseKey(ParserState& state) {
    if (state.data[state.position] == '"') {
        return parseQuotedString(state);
    } else {
        return parseBareString(state);
    }
}

DataElement OneSDataStorage::parseString(ParserState& state) {
    DataElement element;
    element.value = parseQuotedString(state);
    return element;
}

DataElement OneSDataStorage::parseNumber(ParserState& state) {
    std::string number_str;
    
    if (state.data[state.position] == '-') {
        number_str += consumeChar(state);
    }

    while (state.position < state.data.size() && isDigit(state.data[state.position])) {
        number_str += consumeChar(state);
    }

    try {
        DataElement element;
        element.value = static_cast<IntValue>(std::stoll(number_str));
        return element;
    } catch (...) {
        throw std::runtime_error("Invalid number format: " + number_str);
    }
}

DataElement OneSDataStorage::parseIdentifier(ParserState& state) {
    std::string ident = parseBareString(state);
    DataElement element;
    
    if (ident == "#") {
        // Обработка ссылок вида {"#",UUID,number}
        consumeChar(state, ',');
        auto uuid_str = parseBareString(state);
        consumeChar(state, ',');
        auto number = parseNumber(state);
        
        try {
            boost::uuids::string_generator gen;
            UUID uuid = gen(uuid_str);
            element.value = uuid;
            
            // Сохраняем дополнительную информацию в children
            DataElement ref_element;
            ref_element.key = "ref_type";
            ref_element.value = boost::get<IntValue>(number.value);
            element.children.push_back(ref_element);
        } catch (...) {
            element.value = ident + "," + uuid_str;
        }
    } else {
        element.value = ident;
    }
    
    return element;
}

std::string OneSDataStorage::parseQuotedString(ParserState& state) {
    consumeChar(state, '"');
    std::string result;
    bool escape = false;

    while (state.position < state.data.size()) {
        char c = state.data[state.position];
        
        if (escape) {
            result += c;
            escape = false;
            state.position++;
        } else if (c == '\\') {
            escape = true;
            state.position++;
        } else if (c == '"') {
            state.position++;
            return result;
        } else {
            result += c;
            state.position++;
        }
    }
    
    throw std::runtime_error("Unterminated string");
}

std::string OneSDataStorage::parseBareString(ParserState& state) {
    std::string result;

    while (state.position < state.data.size()) {
        char c = state.data[state.position];
        
        if (c == ',' || c == ':' || c == '}' || c == '{' || 
            c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            break;
        }

        result += c;
        state.position++;
    }

    return result;
}

void OneSDataStorage::skipWhitespace(ParserState& state) {
    while (state.position < state.data.size() && 
           (state.data[state.position] == ' ' || 
            state.data[state.position] == '\t' || 
            state.data[state.position] == '\n' || 
            state.data[state.position] == '\r')) {
        if (state.data[state.position] == '\n') {
            state.line++;
            state.column = 1;
        } else {
            state.column++;
        }
        state.position++;
    }
}

char OneSDataStorage::consumeChar(ParserState& state, char expected) {
    if (state.position >= state.data.size()) {
        throw std::runtime_error("Unexpected end of data");
    }
    
    char c = state.data[state.position++];
    state.column++;
    
    if (expected != '\0' && c != expected) {
        std::stringstream ss;
        ss << "Expected '" << expected << "' but found '" << c 
           << "' at line " << state.line << ", column " << state.column;
        throw std::runtime_error(ss.str());
    }
    
    return c;
}

bool OneSDataStorage::isPotentialKey(ParserState& state) {
    size_t saved_pos = state.position;
    try {
        std::string potential_key;
        if (state.data[state.position] == '"') {
            potential_key = parseQuotedString(state);
        } else {
            potential_key = parseBareString(state);
        }
        
        skipWhitespace(state);
        bool is_key = (state.position < state.data.size() && state.data[state.position] == ':');
        state.position = saved_pos;
        return is_key;
    } catch (...) {
        state.position = saved_pos;
        return false;
    }
}

void OneSDataStorage::buildIndexes(DataElement* element) {
    // Индексируем текущий элемент
    if (element->value.type() == typeid(UUID)) {
        uuid_index[boost::get<UUID>(element->value)].push_back(element);
    } else if (element->value.type() == typeid(StringValue)) {
        text_index[boost::get<StringValue>(element->value)].push_back(element);
    } else if (element->value.type() == typeid(IntValue)) {
        number_index[boost::get<IntValue>(element->value)].push_back(element);
    }

    // Рекурсивно индексируем детей
    for (auto& child : element->children) {
        buildIndexes(&child);
    }
}

void OneSDataStorage::serializeElement(const DataElement& element, ptree& tree) const {
    if (element.children.empty()) {
        // Листовой элемент
        if (element.value.type() == typeid(UUID)) {
            tree.put(element.key, boost::uuids::to_string(boost::get<UUID>(element.value)));
        } else if (element.value.type() == typeid(StringValue)) {
            tree.put(element.key, boost::get<StringValue>(element.value));
        } else if (element.value.type() == typeid(IntValue)) {
            tree.put(element.key, boost::get<IntValue>(element.value));
        } else if (element.value.type() == typeid(std::nullptr_t)) {
            tree.put(element.key, "null");
        }
    } else {
        // Составной элемент
        ptree children_tree;
        for (const auto& child : element.children) {
            ptree child_tree;
            serializeElement(child, child_tree);
            if (element.is_array) {
                children_tree.push_back(std::make_pair("", child_tree));
            } else if (!child.key.empty()) {
                children_tree.add_child(child.key, child_tree);
            }
        }
        tree.add_child(element.key.empty() ? "value" : element.key, children_tree);
    }
}

bool OneSDataStorage::isDigit(char c) const { 
    return c >= '0' && c <= '9'; 
}

bool OneSDataStorage::isAlpha(char c) const { 
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'); 
}

// Реализация методов OneSUtils
UUID OneSUtils::parseUUID(const std::string& str) {
    try {
        boost::uuids::string_generator gen;
        return gen(str);
    } catch (...) {
        throw std::runtime_error("Invalid UUID format: " + str);
    }
}

bool OneSUtils::isValidUUID(const std::string& str) {
    if (str.length() != 36) return false;
    for (size_t i = 0; i < str.length(); ++i) {
        char c = str[i];
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            if (c != '-') return false;
        } else if (!std::isxdigit(c)) {
            return false;
        }
    }
    return true;
}

std::string OneSUtils::uuidToString(const UUID& uuid) {
    return boost::uuids::to_string(uuid);
}