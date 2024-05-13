#include "v8MetaDoc.h"

wstring v8MetaDoc::MetaName()
{
    return MetaClassName() + L"." + Name;
}

wstring v8MetaDoc::MetaClassName()
{
    return L"Документы";
}

v8MetaDoc::v8MetaDoc(v8Tree* t)
{
    UUID = L"";
    Name = L"docs";
}

v8MetaDoc::~v8MetaDoc()
{
}

