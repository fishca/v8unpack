#pragma once

#include <string>
#include <list>
#include <iostream>
#include "v8Tree.h"
#include "MetaObject.h"

class v8MetaDoc : public MetaObject
{
public:
	wstring MetaName();
	wstring MetaClassName();

	v8MetaDoc(v8Tree* t);
	~v8MetaDoc();

private:
	wstring UUID;
	wstring Name;

};

