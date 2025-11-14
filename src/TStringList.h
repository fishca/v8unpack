#ifndef TSTRINGLIST_H
#define TSTRINGLIST_H

#include <vector>
#include <string>
#include <functional>
#include <memory>

enum class DuplicatesType {
    dupIgnore,
    dupAccept,
    dupError
};

class TStringList {
private:
    std::vector<std::string> items;
    mutable std::vector<std::string> names;
    mutable std::vector<std::string> values;
    mutable bool nameValueMode = false;
    bool sorted = false;
    bool caseSensitive = true;
    DuplicatesType duplicates = DuplicatesType::dupAccept;

    static std::string ToLower(const std::string& str);
    void UpdateNameValue(int index) const;
    void UpdateAllNameValue() const;

public:
    // Конструкторы и деструктор
    TStringList() = default;
    ~TStringList() = default;
    
    // Базовые операции
    void Add(const std::string& str);
    void Insert(int index, const std::string& str);
    void Delete(int index);
    void Clear();
    
    // Доступ к элементам
    std::string& operator[](int index);
    const std::string& operator[](int index) const;
    std::string GetString(int index) const;
    void PutString(int index, const std::string& value);
    
    // Работа с Name=Value
    std::string GetName(int index) const;
    std::string GetValue(int index) const;
    std::string GetValue(const std::string& name) const;
    void SetValue(const std::string& name, const std::string& value);
    
    // Поиск
    int IndexOf(const std::string& str) const;
    int IndexOfName(const std::string& name) const;
    
    // Сортировка
    void Sort();
    void CustomSort(std::function<bool(const std::string&, const std::string&)> compare);
    
    // Работа с файлами
    void LoadFromFile(const std::string& filename);
    void SaveToFile(const std::string& filename) const;
    
    // Свойства
    int Count() const;
    bool Empty() const;
    
    void SetSorted(bool value);
    bool GetSorted() const;
    
    void SetCaseSensitive(bool value);
    bool GetCaseSensitive() const;
    
    void SetDuplicates(DuplicatesType value);
    DuplicatesType GetDuplicates() const;
    
    // Текст как единая строка
    std::string GetText() const;
    void SetText(const std::string& text);
    
    // Comma-разделенные значения
    std::string GetCommaText() const;
    void SetCommaText(const std::string& text);
    
    // События
    std::function<void(TStringList&)> onChange;
    std::function<void(TStringList&)> onAdd;
    std::function<void(TStringList&)> onDelete;
    std::function<void(TStringList&)> onClear;
    
    // Итераторы для STL-совместимости
    auto begin() -> decltype(items.begin());
    auto end() -> decltype(items.end());
    auto begin() const -> decltype(items.begin());
    auto end() const -> decltype(items.end());

};

#endif // TSTRINGLIST_H