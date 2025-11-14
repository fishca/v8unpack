#ifndef MDFORM_H
#define MDFORM_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "TMStree.h"
#include "StringUtils.h"


// Предварительные объявления (если не определены в этом файле)
class TStorageReader;
class TStorageReader64;

class mdForm
{
private:
    std::string FUUID;
    std::string FName;

public:
    // Геттеры вместо properties
    const std::string& UUID() const { return FUUID; }
    const std::string& Name() const { return FName; }

    // Конструкторы
    mdForm() = default;
    mdForm(const std::string& strUUID);
    

    // Методы
    std::string MetaName();
    void FillUUID(std::unordered_set<std::string>& UUIDList);
};

class mdFormList
{
private:
    std::vector<std::unique_ptr<mdForm>> FList;

    mdForm* Get(int Index);
    void Put(int Index, mdForm* Item);

public:
    // Доступ к элементам — как default property в Delphi
    mdForm* operator[](int Index) { return Get(Index); }
    const mdForm* operator[](int Index) const { return const_cast<mdFormList*>(this)->Get(Index); }

    // Конструкторы
    mdFormList(const std::string& StrUUID);

    // Утилиты
    size_t Count() const { return FList.size(); }
    void Add(mdForm* Item); // если понадобится позже

    // Запрещаем копирование (т.к. unique_ptr non-copyable)
    mdFormList(const mdFormList&) = delete;
    mdFormList& operator=(const mdFormList&) = delete;

    // Перемещение разрешено (по умолчанию)
    mdFormList(mdFormList&&) = default;
    mdFormList& operator=(mdFormList&&) = default;

    ~mdFormList() = default; // unique_ptr сам всё удалит
};


#endif  // MDFORM_H
