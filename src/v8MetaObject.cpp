#include "v8MetaObject.h"

void v8MetaObject::SetName(string aName)
{
	Name = aName;
}

string v8MetaObject::GetName()
{
	return Name;
}

void v8MetaObject::SetMetaClassName(string aClassName)
{
	ClassName = aClassName;
}

string v8MetaObject::GetMetaClassName()
{
	return ClassName;
}

void v8MetaObject::SetUUID(string guid)
{
	UUID = guid;
}

string v8MetaObject::GetUUID()
{
	return UUID;
}

void v8MetaObject::FillUUID()
{
	UUID = ' ';
}

v8MetaObject::v8MetaObject()
{
	this->ClassName = "ClassMetaObject";
	this->Name = "MetaObject";
	this->UUID = "00000000-0000-0000-0000-000000000000";
}

v8MetaObject::~v8MetaObject()
{
}
