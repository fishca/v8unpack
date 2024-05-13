#pragma once

#include <string>
#include <list>
#include <iostream>
#include "v8Tree.h"
#include "MetaObject.h"

using namespace std;

enum CommandMode
{
	cmMain,
	cmRef,
	cmBP

};

class v8MetaCommand : public MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName();

	void FillUUID(list<wstring> UUIDList);

	wstring getName();
	wstring getUUID();

	v8MetaCommand(v8Tree* t);
	~v8MetaCommand();

private:
	wstring UUID;
	wstring Name;

};

