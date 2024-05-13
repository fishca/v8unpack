#pragma once

#include <string>
#include <list>
#include <iostream>
#include "v8Tree.h"
#include "MetaObject.h"

using namespace std;


class v8MetaForm : public MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName() = delete;

	void FillUUID(list<wstring> UUIDList);

	wstring getName();
	wstring getUUID();

	v8MetaForm(v8Tree* t);
	~v8MetaForm();

private:
	wstring UUID;
	wstring Name;
};

