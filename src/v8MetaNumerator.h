#pragma once
#include "MetaObject.h"
class v8MetaNumerator : public MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName();

	v8MetaNumerator(v8Tree* t);
	~v8MetaNumerator();

private:
	wstring UUID;
	wstring Name;
	
};

