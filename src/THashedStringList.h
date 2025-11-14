#ifndef HASHED_STRING_LIST_H
#define HASHED_STRING_LIST_H

#include <string>
#include <vector>
#include <unordered_map>
#include <utility>

class THashedStringList
{
public:
    // Конструктор
    THashedStringList();

    // Добавить строку вида "Key=Value"
    void Add(const std::string& line);

    // Установить значение по ключу (добавляет или заменяет)
    void SetValue(const std::string& key, const std::string& value);

    // Получить значение по ключу; возвращает пустую строку, если ключ не найден
    std::string GetValue(const std::string& key) const;

    // Проверить, существует ли ключ
    bool HasKey(const std::string& key) const;

    // Удалить запись по ключу
    bool RemoveKey(const std::string& key);

    // Очистить весь список
    void Clear();

    // Получить количество записей
    size_t Count() const;

    // Получить строку по индексу (в порядке добавления)
    std::string GetLine(size_t index) const;

    // Получить ключ по индексу
    std::string GetKey(size_t index) const;

    // Получить значение по индексу
    std::string GetValueByIndex(size_t index) const;

private:
    // Разделить строку на ключ и значение по символу '='
    static std::pair<std::string, std::string> SplitLine(const std::string& line);

    // Хранит порядок вставки
    std::vector<std::pair<std::string, std::string>> m_lines;

    // Хеш-таблица для быстрого поиска
    std::unordered_map<std::string, std::string> m_map;
};

#endif // HASHED_STRING_LIST_H