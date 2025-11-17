#ifndef MDLANG_H
#define MDLANG_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "THashedStringList.h"
#include "EctoSoftTree.h"
#include "mdObject.h"
#include "TMSTree.h"



class mdLang: public mdObject
{
public:
    // ������������
    mdLang() = default;
    mdLang(const std::string strUUID);
    

    // ������
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
    // ������ � ��������� � ��� default property � Delphi
    mdLang* operator[](int Index) { return Get(Index); }
    const mdLang* operator[](int Index) const { return const_cast<mdLangList*>(this)->Get(Index); }

    // ������������
    mdLangList(TMSTree MetaTree);
    

    // �������
    size_t Count() const { return FList.size(); }
    void Add(mdLang* Item); // ���� ����������� �����

    // ��������� ����������� (�.�. unique_ptr non-copyable)
    mdLangList(const mdLangList&) = delete;
    mdLangList& operator=(const mdLangList&) = delete;

    // ����������� ��������� (�� ���������)
    mdLangList(mdLangList&&) = default;
    mdLangList& operator=(mdLangList&&) = default;

    ~mdLangList() = default; // unique_ptr ��� �� ������
};


#endif  // MDLANG_H
