#ifndef MDMOXEL_H
#define MDMOXEL_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>

// Предварительные объявления (если не определены в этом файле)
class TStorageReader;
class TStorageReader64;

class mdMoxel
{
private:
    std::string FUUID;
    std::string FName;

public:
    // Геттеры вместо properties
    const std::string& UUID() const { return FUUID; }
    const std::string& Name() const { return FName; }

    // Конструкторы
    mdMoxel(const std::string& strUUID, TStorageReader* MetaDataReader);
    mdMoxel(const std::string& strUUID, TStorageReader64* MetaDataReader);

    // Методы
    std::string MetaName();
    void FillUUID(std::unordered_set<std::string>& UUIDList);
};

class mdMoxelList
{
private:
    std::vector<std::unique_ptr<mdMoxel>> FList;

    mdMoxel* Get(int Index);
    void Put(int Index, mdMoxel* Item);

public:
    // Доступ к элементам — как default property в Delphi
    mdMoxel* operator[](int Index) { return Get(Index); }
    const mdMoxel* operator[](int Index) const { return const_cast<mdMoxelList*>(this)->Get(Index); }

    // Конструкторы
    mdMoxelList(const std::string& StrUUID, TStorageReader* MetaDataReader);
    mdMoxelList(const std::string& StrUUID, TStorageReader64* MetaDataReader);

    // Утилиты
    size_t Count() const { return FList.size(); }
    void Add(mdMoxel* Item); // если понадобится позже

    // Запрещаем копирование (т.к. unique_ptr non-copyable)
    mdMoxelList(const mdMoxelList&) = delete;
    mdMoxelList& operator=(const mdMoxelList&) = delete;

    // Перемещение разрешено (по умолчанию)
    mdMoxelList(mdMoxelList&&) = default;
    mdMoxelList& operator=(mdMoxelList&&) = default;

    ~mdMoxelList() = default; // unique_ptr сам всё удалит
};


#endif  // MDMOXEL_H
