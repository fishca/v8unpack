#include "v8MetaCommonModules.h"

wstring v8MetaCommonModules::MetaName()
{
	return MetaClassName() + L"." + Name;
}

wstring v8MetaCommonModules::MetaClassName()
{
	return L"Общие модули";
}

v8MetaCommonModules::v8MetaCommonModules(v8Tree* t)
{
	UUID = L"";
	Name = L"";
}

v8MetaCommonModules::~v8MetaCommonModules()
{
}
