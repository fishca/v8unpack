#include "MetaObject.h"


wstring MetaObject::MetaName()
{
	return L"Объект." + Name;
}

wstring MetaObject::MetaClassName()
{
	return L"Объект";
}

void MetaObject::FillUUID(list<wstring> UUIDList)
{
}

wstring MetaObject::getName()
{
	return Name;
}

wstring MetaObject::getUUID()
{
	return UUID;
}

MetaObject::MetaObject(v8Tree* t)
{
	UUID = L"00000000-0000-0000-0000-000000000000";
	Name = L"Объект";
}

MetaObject::~MetaObject()
{
}
