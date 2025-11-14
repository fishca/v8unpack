#include "TStringList.h"
#include <algorithm>
#include <fstream>
#include <stdexcept>
#include <cctype>

// Базовые операции
void TStringList::Add(const std::string& str) {
    if (duplicates == DuplicatesType::dupIgnore) {
        if (IndexOf(str) != -1) return;
    } else if (duplicates == DuplicatesType::dupError) {
        if (IndexOf(str) != -1) {
            throw std::runtime_error("Duplicate string not allowed");
        }
    }
    
    items.push_back(str);
    UpdateNameValue(static_cast<int>(items.size() - 1));
    SetSorted(false);
    
    if (onChange) onChange(*this);
    if (onAdd) onAdd(*this);
}

void TStringList::Insert(int index, const std::string& str) {
    if (index < 0 || index > static_cast<int>(items.size())) {
        throw std::out_of_range("Index out of range");
    }
    
    if (duplicates == DuplicatesType::dupIgnore) {
        if (IndexOf(str) != -1) return;
    } else if (duplicates == DuplicatesType::dupError) {
        if (IndexOf(str) != -1) {
            throw std::runtime_error("Duplicate string not allowed");
        }
    }
    
    items.insert(items.begin() + index, str);
    UpdateNameValue(index);
    SetSorted(false);
    
    if (onChange) onChange(*this);
}

void TStringList::Delete(int index) {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        throw std::out_of_range("Index out of range");
    }
    
    items.erase(items.begin() + index);
    if (nameValueMode) {
        names.erase(names.begin() + index);
        values.erase(values.begin() + index);
    }
    
    if (onChange) onChange(*this);
    if (onDelete) onDelete(*this);
}

void TStringList::Clear() {
    items.clear();
    names.clear();
    values.clear();
    sorted = false;
    
    if (onChange) onChange(*this);
    if (onClear) onClear(*this);
}

// Доступ к элементам
std::string& TStringList::operator[](int index) {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        throw std::out_of_range("Index out of range");
    }
    return items[index];
}

const std::string& TStringList::operator[](int index) const {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        throw std::out_of_range("Index out of range");
    }
    return items[index];
}

std::string TStringList::GetString(int index) const {
    return (*this)[index];
}

void TStringList::PutString(int index, const std::string& value) {
    if (index < 0 || index >= static_cast<int>(items.size())) {
        throw std::out_of_range("Index out of range");
    }
    
    items[index] = value;
    UpdateNameValue(index);
    SetSorted(false);
    
    if (onChange) onChange(*this);
}

// Работа с Name=Value
std::string TStringList::GetName(int index) const {
    if (!nameValueMode) UpdateAllNameValue();
    if (index < 0 || index >= static_cast<int>(names.size())) {
        throw std::out_of_range("Index out of range");
    }
    return names[index];
}

std::string TStringList::GetValue(int index) const {
    if (!nameValueMode) UpdateAllNameValue();
    if (index < 0 || index >= static_cast<int>(values.size())) {
        throw std::out_of_range("Index out of range");
    }
    return values[index];
}

std::string TStringList::GetValue(const std::string& name) const {
    int index = IndexOfName(name);
    return (index != -1) ? GetValue(index) : "";
}

void TStringList::SetValue(const std::string& name, const std::string& value) {
    int index = IndexOfName(name);
    if (index != -1) {
        PutString(index, name + "=" + value);
    } else {
        Add(name + "=" + value);
    }
}

// Поиск
int TStringList::IndexOf(const std::string& str) const {
    if (sorted && caseSensitive) {
        auto it = std::lower_bound(items.begin(), items.end(), str);
        if (it != items.end() && *it == str) {
            return static_cast<int>(std::distance(items.begin(), it));
        }
        return -1;
    } else if (sorted && !caseSensitive) {
        std::string lowerStr = ToLower(str);
        auto it = std::lower_bound(items.begin(), items.end(), lowerStr,
            [](const std::string& a, const std::string& b) {
                return ToLower(a) < ToLower(b);
            });
        if (it != items.end() && ToLower(*it) == lowerStr) {
            return static_cast<int>(std::distance(items.begin(), it));
        }
        return -1;
    } else {
        for (size_t i = 0; i < items.size(); ++i) {
            if (caseSensitive) {
                if (items[i] == str) return static_cast<int>(i);
            } else {
                if (ToLower(items[i]) == ToLower(str)) return static_cast<int>(i);
            }
        }
        return -1;
    }
}

int TStringList::IndexOfName(const std::string& name) const {
    if (!nameValueMode) UpdateAllNameValue();
    
    for (size_t i = 0; i < names.size(); ++i) {
        if (caseSensitive) {
            if (names[i] == name) return static_cast<int>(i);
        } else {
            if (ToLower(names[i]) == ToLower(name)) return static_cast<int>(i);
        }
    }
    return -1;
}

// Сортировка
void TStringList::Sort() {
    if (sorted) return;
    
    if (caseSensitive) {
        std::sort(items.begin(), items.end());
    } else {
        std::sort(items.begin(), items.end(),
            [](const std::string& a, const std::string& b) {
                return ToLower(a) < ToLower(b);
            });
    }
    
    sorted = true;
    if (onChange) onChange(*this);
}

void TStringList::CustomSort(std::function<bool(const std::string&, const std::string&)> compare) {
    std::sort(items.begin(), items.end(), compare);
    SetSorted(false);
    if (onChange) onChange(*this);
}

// Работа с файлами
void TStringList::LoadFromFile(const std::string& filename) {
    Clear();
    
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    std::string line;
    while (std::getline(file, line)) {
        // Убираем CR в конце строки для Windows
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        Add(line);
    }
    
    file.close();
}

void TStringList::SaveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot create file: " + filename);
    }
    
    for (const auto& item : items) {
        file << item << "\n";
    }
    
    file.close();
}

// Свойства
int TStringList::Count() const { 
    return static_cast<int>(items.size()); 
}

bool TStringList::Empty() const { 
    return items.empty(); 
}

void TStringList::SetSorted(bool value) { 
    sorted = value; 
    if (sorted) Sort();
}

bool TStringList::GetSorted() const { 
    return sorted; 
}

void TStringList::SetCaseSensitive(bool value) { 
    caseSensitive = value; 
    SetSorted(false);
}

bool TStringList::GetCaseSensitive() const { 
    return caseSensitive; 
}

void TStringList::SetDuplicates(DuplicatesType value) { 
    duplicates = value; 
}

DuplicatesType TStringList::GetDuplicates() const { 
    return duplicates; 
}

// Текст как единая строка
std::string TStringList::GetText() const {
    std::string result;
    for (size_t i = 0; i < items.size(); ++i) {
        result += items[i];
        if (i < items.size() - 1) result += "\r\n";
    }
    return result;
}

void TStringList::SetText(const std::string& text) {
    Clear();
    
    std::string line;
    for (char c : text) {
        if (c == '\n') {
            Add(line);
            line.clear();
        } else if (c != '\r') {
            line += c;
        }
    }
    
    if (!line.empty()) {
        Add(line);
    }
}

// Comma-разделенные значения
std::string TStringList::GetCommaText() const {
    std::string result;
    for (size_t i = 0; i < items.size(); ++i) {
        result += '"' + items[i] + '"';
        if (i < items.size() - 1) result += ',';
    }
    return result;
}

void TStringList::SetCommaText(const std::string& text) {
    Clear();
    
    std::string current;
    bool inQuotes = false;
    
    for (size_t i = 0; i < text.length(); ++i) {
        char c = text[i];
        
        if (c == '"') {
            inQuotes = !inQuotes;
        } else if (c == ',' && !inQuotes) {
            Add(current);
            current.clear();
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        Add(current);
    }
}

// Итераторы для STL-совместимости
auto TStringList::begin() -> decltype(items.begin()) { 
    return items.begin(); 
}

auto TStringList::end() -> decltype(items.end()) { 
    return items.end(); 
}

auto TStringList::begin() const -> decltype(items.begin()) { 
    return items.begin(); 
}

auto TStringList::end() const -> decltype(items.end()) { 
    return items.end(); 
}

// Приватные методы
std::string TStringList::ToLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

void TStringList::UpdateNameValue(int index) const {
    if (index < 0 || index >= static_cast<int>(items.size())) return;
    
    if (names.size() != items.size()) names.resize(items.size());
    if (values.size() != items.size()) values.resize(items.size());
    
    const std::string& item = items[index];
    size_t pos = item.find('=');
    if (pos != std::string::npos) {
        names[index] = item.substr(0, pos);
        values[index] = item.substr(pos + 1);
    } else {
        names[index] = item;
        values[index] = "";
    }
    
    nameValueMode = true;
}

void TStringList::UpdateAllNameValue() const {
    names.resize(items.size());
    values.resize(items.size());
    
    for (size_t i = 0; i < items.size(); ++i) {
        const std::string& item = items[i];
        size_t pos = item.find('=');
        if (pos != std::string::npos) {
            names[i] = item.substr(0, pos);
            values[i] = item.substr(pos + 1);
        } else {
            names[i] = item;
            values[i] = "";
        }
    }
    
    nameValueMode = true;
}