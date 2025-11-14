#include "mdMoxel.h"
#include <stdexcept> // если нужно кидать исключения при отсутствии данных

// Реализация конструкторов
mdMoxel::mdMoxel(const std::string& strUUID, TStorageReader* MetaDataReader)
    : FUUID(strUUID)
{
    if (MetaDataReader == nullptr)
        throw std::invalid_argument("MetaDataReader cannot be null");

    // Здесь должна быть логика чтения FName из MetaDataReader
    // Например:
    // FName = MetaDataReader->ReadName(FUUID);
}

mdMoxel::mdMoxel(const std::string& strUUID, TStorageReader64* MetaDataReader)
    : FUUID(strUUID)
{
    if (MetaDataReader == nullptr)
        throw std::invalid_argument("MetaDataReader64 cannot be null");

    // Здесь должна быть логика чтения FName из MetaDataReader64
    // Например:
    // FName = MetaDataReader64->ReadName(FUUID);
}

std::string mdMoxel::MetaName()
{
    // В оригинале не указано, что делает MetaName.
    // Часто это возвращает имя формы или UUID, если имя пустое.
    // Пример реализации:
    return FName.empty() ? FUUID : FName;
}

void mdMoxel::FillUUID(std::unordered_set<std::string>& UUIDList)
{
    UUIDList.insert(FUUID);
}

/////////////////////////////////////////////////// mdMoxelList ////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
mdMoxel* mdMoxelList::Get(int Index)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    return FList[Index].get();
}

void mdMoxelList::Put(int Index, mdMoxel* Item)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    if (Item == nullptr)
        throw std::invalid_argument("Item cannot be null");

    FList[Index].reset(Item); // старый объект будет удалён автоматически
}

mdMoxelList::mdMoxelList(const std::string& StrUUID, TStorageReader* MetaDataReader)
{
    if (!MetaDataReader)
        throw std::invalid_argument("MetaDataReader cannot be null");

    // Пример логики: читаем список дочерних UUID'ов и создаём TMetaForm для каждого
    // Это псевдокод — адаптируйте под вашу логику
    // auto childUUIDs = MetaDataReader->GetChildUUIDs(StrUUID);
    // for (const auto& uuid : childUUIDs) {
    //     FList.push_back(std::make_unique<TMetaForm>(uuid, MetaDataReader));
    // }

    // Пока оставим пустым или добавим одну форму, если это уместно
    // Например, если StrUUID — корневой, и нужно прочитать одну форму:
    FList.push_back(std::make_unique<mdMoxel>(StrUUID, MetaDataReader));
}

mdMoxelList::mdMoxelList(const std::string& StrUUID, TStorageReader64* MetaDataReader)
{
    if (!MetaDataReader)
        throw std::invalid_argument("MetaDataReader64 cannot be null");

    // Аналогично
    FList.push_back(std::make_unique<mdMoxel>(StrUUID, MetaDataReader));
}

// Дополнительно (если нужно добавлять вручную)
void mdMoxelList::Add(mdMoxel* Item)
{
    if (!Item)
        throw std::invalid_argument("Item cannot be null");
    FList.push_back(std::unique_ptr<mdMoxel>(Item));
}