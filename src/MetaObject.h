#pragma once

#include <string>
#include <list>
#include <iostream>
#include "v8Tree.h"

using namespace std;


class MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName();

	void FillUUID(list<wstring> UUIDList);

	wstring getName();
	wstring getUUID();

	MetaObject() {}
	MetaObject(v8Tree* t);
	virtual ~MetaObject();

private:
	wstring UUID;
	wstring Name;

};

