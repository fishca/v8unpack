#pragma once

#include <string>
#include <list>
#include <iostream>
#include "v8Tree.h"
#include "MetaObject.h"

using namespace std;

class v8MetaMoxel : public MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName();

	void FillUUID(list<wstring> UUIDList);

	wstring getName();
	wstring getUUID();

	v8MetaMoxel(v8Tree* t);
	~v8MetaMoxel();

private:
	wstring UUID;
	wstring Name;
};

