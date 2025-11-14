#ifndef MDCOMMAND_H
#define MDCOMMAND_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "THashedStringList.h"
#include "EctoSoftTree.h"

// Предварительные объявления (если не определены в этом файле)
class TStorageReader;
class TStorageReader64;

enum class TCommandMode { 
    cmMain, 
    cmRef, 
    cmBP 
};

class mdCommand
{
private:
    std::string FUUID;
    std::string FName;

public:
    // Геттеры вместо properties
    const std::string& UUID() const { return FUUID; }
    const std::string& Name() const { return FName; }

    // Конструкторы
    mdCommand(EctoTreeNode* Root);
    

    // Методы
    std::string MetaName();
    std::string MetaClassName();
    void FillUUID(THashedStringList UUIDList);
};

class mdCommandList
{
private:
    std::vector<std::unique_ptr<mdCommand>> FList;

    mdCommand* Get(int Index);
    void Put(int Index, mdCommand* Item);

public:
    // Доступ к элементам — как default property в Delphi
    mdCommand* operator[](int Index) { return Get(Index); }
    const mdCommand* operator[](int Index) const { return const_cast<mdCommandList*>(this)->Get(Index); }

    // Конструкторы
    mdCommandList(EctoTreeNode* Root, TCommandMode Mode);
    

    // Утилиты
    size_t Count() const { return FList.size(); }
    void Add(mdCommand* Item); // если понадобится позже

    // Запрещаем копирование (т.к. unique_ptr non-copyable)
    mdCommandList(const mdCommandList&) = delete;
    mdCommandList& operator=(const mdCommandList&) = delete;

    // Перемещение разрешено (по умолчанию)
    mdCommandList(mdCommandList&&) = default;
    mdCommandList& operator=(mdCommandList&&) = default;

    ~mdCommandList() = default; // unique_ptr сам всё удалит
};


#endif  // MDCOMMAND_H
