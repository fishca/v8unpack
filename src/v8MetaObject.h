#pragma once


#include <string>
#include <iostream>

using namespace std;

class v8MetaObject
{
public:
	void SetName(string aName);
	string GetName();
	
	void SetMetaClassName(string aClassName);
	string GetMetaClassName();

	void SetUUID(string guid);
	string GetUUID();

	void FillUUID();
	
	v8MetaObject();
	virtual ~v8MetaObject();

private:
	string UUID;
	string Name;
	string ClassName;
};

