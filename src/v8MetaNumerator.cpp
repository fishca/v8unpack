#include "v8MetaNumerator.h"

wstring v8MetaNumerator::MetaName()
{
    return MetaClassName() + L"." + Name;
}

wstring v8MetaNumerator::MetaClassName()
{
    return L"Нумератор";
}

v8MetaNumerator::v8MetaNumerator(v8Tree* t)
{
    UUID = L"";
    Name = L"";
}

v8MetaNumerator::~v8MetaNumerator()
{
}
