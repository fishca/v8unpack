#ifndef MDFORM_H
#define MDFORM_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "TMSTree.h"
#include "StringUtils.h"


// ��������������� ���������� (���� �� ���������� � ���� �����)
class TStorageReader;
class TStorageReader64;

class mdForm
{
private:
    std::string FUUID;
    std::string FName;

public:
    // ������� ������ properties
    const std::string& UUID() const { return FUUID; }
    const std::string& Name() const { return FName; }

    // ������������
    mdForm() = default;
    mdForm(const std::string& strUUID);
    

    // ������
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
    // ������ � ��������� � ��� default property � Delphi
    mdForm* operator[](int Index) { return Get(Index); }
    const mdForm* operator[](int Index) const { return const_cast<mdFormList*>(this)->Get(Index); }

    // ������������
    mdFormList(const std::string& StrUUID);

    // �������
    size_t Count() const { return FList.size(); }
    void Add(mdForm* Item); // ���� ����������� �����

    // ��������� ����������� (�.�. unique_ptr non-copyable)
    mdFormList(const mdFormList&) = delete;
    mdFormList& operator=(const mdFormList&) = delete;

    // ����������� ��������� (�� ���������)
    mdFormList(mdFormList&&) = default;
    mdFormList& operator=(mdFormList&&) = default;

    ~mdFormList() = default; // unique_ptr ��� �� ������
};


#endif  // MDFORM_H
