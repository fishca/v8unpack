#ifndef MDOBJECT_H
#define MDOBJECT_H
#include <string>
#include <memory>
#include <vector>
#include "mdForm.h"
#include "mdMoxel.h"


class TStorageReader;
class TStorageReader64;
class TMetaFormList;
class TMetaMoxelList;
class TMetaCommandList;
class THashedStringList;

class mdObject
{
protected:
    std::string FUUID;
    std::string FName;
    
    //std::unique_ptr<TMetaFormList> FForms;
    //std::unique_ptr<TMetaMoxelList> FMoxels;
    //std::unique_ptr<TMetaCommandList> FCommands;
    TMetaFormList* FForms;
    TMetaMoxelList* FMoxels;
    TMetaCommandList* FCommands;

    //void SetCommands(TMetaCommandList* Value);

public:
    // Конструкторы
    mdObject() = default;
    mdObject(const std::string& strUUID, const std::string& FormsUUID);
    // Деструктор
    virtual ~mdObject();

    // Свойства
    TMetaFormList* Forms;
    TMetaMoxelList* Moxels;
    TMetaCommandList* Commands;
    std::string UUID;
    std::string Name;

    // Чтение свойств через геттеры (рекомендуемый подход в C++)
    TMetaFormList* GetForms() const { return FForms; }
    void SetForms(TMetaFormList* forms) { FForms = forms; }

    TMetaMoxelList* GetMoxels() const { return FMoxels; }
    void SetMoxels(TMetaMoxelList* moxels) { FMoxels = moxels; }

    TMetaCommandList* GetCommands() const { return FCommands; }
    void SetCommands(TMetaCommandList* commands) { SetCommands(commands); }

    std::string GetUUID() const { return FUUID; }
    void SetUUID(const std::string& uuid) { FUUID = uuid; }

    std::string GetName() const { return FName; }
    void SetName(const std::string& name) { FName = name; }

    // Чисто виртуальные методы (аналоги abstract в Delphi)
    virtual std::string MetaName() = 0;
    virtual std::string MetaClassName() = 0;

    // Виртуальный метод (аналог dynamic в Delphi)
    virtual void FillUUID(THashedStringList* UUIDList,
        THashedStringList* UUIDClassList,
        THashedStringList* FormsList,
        THashedStringList* MoxelsList);

    // Удаляем копирование и присваивание для безопасности
    mdObject(const mdObject&) = delete;
    mdObject& operator=(const mdObject&) = delete;
};

#endif // MDOBJECT_H
