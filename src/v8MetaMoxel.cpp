#include "v8MetaMoxel.h"

wstring v8MetaMoxel::MetaName()
{
    return L"Макет." + Name;
}

wstring v8MetaMoxel::MetaClassName()
{
    return L"Макет";
}

void v8MetaMoxel::FillUUID(list<wstring> UUIDList)
{
    UUIDList.push_back(UUID + L"=" + MetaName());
}

wstring v8MetaMoxel::getName()
{
    return Name;
}

wstring v8MetaMoxel::getUUID()
{
    return UUID;
}

v8MetaMoxel::v8MetaMoxel(v8Tree* t)
{
    UUID = L"";
    Name = L"";
}

v8MetaMoxel::~v8MetaMoxel()
{
}
