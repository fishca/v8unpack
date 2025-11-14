#ifndef TMETA_OBJECTS_H
#define TMETA_OBJECTS_H

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>
#include "THashedStringList.h"

class TStorageReader;
class TStorageReader64;


enum class EntityType { Form, Moxel, Command };

template<EntityType T>
class mdEntity
{
private:
    std::string FUUID;
    std::string FName;

    void loadFrom(TStorageReader* reader);
    //void loadFrom(TStorageReader64* reader);

public:
    mdEntity(const std::string& strUUID, TStorageReader* reader)
        : FUUID(strUUID) {
        loadFrom(reader);
    }

    mdEntity(const std::string& strUUID, TStorageReader64* reader)
        : FUUID(strUUID) {
        loadFrom(reader);
    }

    const std::string& UUID() const { return FUUID; }
    const std::string& Name() const { return FName; }

    std::string MetaName() const { return FName.empty() ? FUUID : FName; }
    
    //void FillUUID(std::unordered_set<std::string>& list) const { 
    //    list.insert(FUUID); 
    //}

    void FillUUID(THashedStringList& list) const {
        list.Add(FUUID + "=" + MetaName);
    }

};

template<>
void mdEntity<EntityType::Form>::loadFrom(TStorageReader* r) {
    FName = "Форма." + FName;
}

template<>
void mdEntity<EntityType::Moxel>::loadFrom(TStorageReader* r) {
    FName = "Макет." + FName;
}

template<>
void mdEntity<EntityType::Command>::loadFrom(TStorageReader* r) {
    FName = "Команды." + FName;
}

#endif  // TMETA_OBJECTS_H

