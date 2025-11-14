#ifndef ONES_DATA_STORAGE_HPP
#define ONES_DATA_STORAGE_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <unordered_map>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/variant.hpp>
#include <boost/unordered_map.hpp>

using namespace boost::property_tree;

// Базовые типы данных
using UUID = boost::uuids::uuid;
using IntValue = int64_t;
using StringValue = std::string;
using BinaryData = std::vector<uint8_t>;

// Вариант для хранения различных типов значений
using ValueVariant = boost::variant<
    IntValue,
    StringValue,
    UUID,
    std::nullptr_t
>;

// Структура для элемента массива/объекта
struct DataElement {
    std::string key;
    ValueVariant value;
    std::vector<DataElement> children;
    bool is_array = false;
    
    DataElement() = default;
    DataElement(std::string k, ValueVariant v);
};

// Структура для хранения ссылок и идентификаторов
struct ObjectReference {
    UUID uuid;
    IntValue version;
    IntValue type;
    
    ObjectReference();
    ObjectReference(UUID id, IntValue ver, IntValue t);
};

// Основной класс для хранения сложных структур 1С
class OneSDataStorage {
private:
    // Быстрые индексы для поиска
    boost::unordered_map<UUID, std::vector<DataElement*>> uuid_index;
    boost::unordered_map<std::string, std::vector<DataElement*>> text_index;
    boost::unordered_map<IntValue, std::vector<DataElement*>> number_index;
    
    std::vector<std::unique_ptr<DataElement>> data_pool;
    DataElement root_element;

    // Внутренняя структура для состояния парсера
    struct ParserState {
        std::string data;
        size_t position = 0;
        size_t line = 1;
        size_t column = 1;
    };

    // Методы парсинга
    DataElement parseElement(ParserState& state);
    DataElement parseObjectOrArray(ParserState& state);
    std::string parseKey(ParserState& state);
    DataElement parseString(ParserState& state);
    DataElement parseNumber(ParserState& state);
    DataElement parseIdentifier(ParserState& state);
    std::string parseQuotedString(ParserState& state);
    std::string parseBareString(ParserState& state);
    
    // Вспомогательные методы парсинга
    void skipWhitespace(ParserState& state);
    char consumeChar(ParserState& state, char expected = '\0');
    bool isPotentialKey(ParserState& state);
    
    // Методы индексации и сериализации
    void buildIndexes(DataElement* element);
    void serializeElement(const DataElement& element, ptree& tree) const;
    
    // Вспомогательные функции
    bool isDigit(char c) const;
    bool isAlpha(char c) const;

public:
    // Конструктор и деструктор
    OneSDataStorage();
    ~OneSDataStorage();

    // Загрузка данных
    bool loadFromString(const std::string& data);
    
    // Методы поиска
    std::vector<DataElement*> findByUUID(const UUID& uuid);
    std::vector<DataElement*> findByText(const std::string& text);
    std::vector<DataElement*> findByNumber(IntValue number);
    
    // Экспорт и сериализация
    ptree toPropertyTree() const;
    void exportToJson(const std::string& filename);
    
    // Утилиты
    const DataElement& getRoot() const { return root_element; }
    size_t getTotalElements() const;
};

// Вспомогательные утилиты
class OneSUtils {
public:
    static UUID parseUUID(const std::string& str);
    static bool isValidUUID(const std::string& str);
    static std::string uuidToString(const UUID& uuid);
};

#endif // ONES_DATA_STORAGE_HPP