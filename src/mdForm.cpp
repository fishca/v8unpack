#include "mdForm.h"
#include <stdexcept> // если нужно кидать исключения при отсутствии данных

// Реализация конструкторов
mdForm::mdForm(const std::string& strUUID) : FUUID(strUUID)
{
}


std::string mdForm::MetaName()
{
    // В оригинале не указано, что делает MetaName.
    // Часто это возвращает имя формы или UUID, если имя пустое.
    // Пример реализации:
    return FName.empty() ? FUUID : FName;
}

void mdForm::FillUUID(std::unordered_set<std::string>& UUIDList)
{
    UUIDList.insert(FUUID);
}

/////////////////////////////////////////////////// mdFormList ////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
mdForm* mdFormList::Get(int Index)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    return FList[Index].get();
}

void mdFormList::Put(int Index, mdForm* Item)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    if (Item == nullptr)
        throw std::invalid_argument("Item cannot be null");

    FList[Index].reset(Item); // старый объект будет удалён автоматически
}

mdFormList::mdFormList(const std::string& StrUUID)
{
    FList.push_back(std::make_unique<mdForm>(StrUUID));
}


// Дополнительно (если нужно добавлять вручную)
void mdFormList::Add(mdForm* Item)
{
    if (!Item)
        throw std::invalid_argument("Item cannot be null");
    FList.push_back(std::unique_ptr<mdForm>(Item));
}