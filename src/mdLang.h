#ifndef MDLANG_H
#define MDLANG_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "THashedStringList.h"
#include "EctoSoftTree.h"
#include "mdObject.h"
#include "TMStree.h"



class mdLang: public mdObject
{
public:
    // Конструкторы
    mdLang() = default;
    mdLang(const std::string strUUID);
    

    // Методы
    std::string MetaName();
    std::string MetaClassName();
};

class mdLangList
{
private:
    std::vector<std::unique_ptr<mdLang>> FList;

    mdLang* Get(int Index);
    void Put(int Index, mdLang* Item);

public:
    // Доступ к элементам — как default property в Delphi
    mdLang* operator[](int Index) { return Get(Index); }
    const mdLang* operator[](int Index) const { return const_cast<mdLangList*>(this)->Get(Index); }

    // Конструкторы
    mdLangList(TMSTree MetaTree);
    

    // Утилиты
    size_t Count() const { return FList.size(); }
    void Add(mdLang* Item); // если понадобится позже

    // Запрещаем копирование (т.к. unique_ptr non-copyable)
    mdLangList(const mdLangList&) = delete;
    mdLangList& operator=(const mdLangList&) = delete;

    // Перемещение разрешено (по умолчанию)
    mdLangList(mdLangList&&) = default;
    mdLangList& operator=(mdLangList&&) = default;

    ~mdLangList() = default; // unique_ptr сам всё удалит
};


#endif  // MDLANG_H
