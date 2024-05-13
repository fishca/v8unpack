#pragma once

#include <string>
#include <list>
#include <iostream>
#include "v8Tree.h"
#include "MetaObject.h"

class v8MetaCommonModules : public MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName();

	v8MetaCommonModules(v8Tree* t);
	~v8MetaCommonModules();

private:
	wstring UUID;
	wstring Name;
};

