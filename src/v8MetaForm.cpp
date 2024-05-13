#include "v8MetaForm.h"

wstring v8MetaForm::MetaName()
{
    return L"Форма." + Name;
}

void v8MetaForm::FillUUID(list<wstring> UUIDList)
{
    UUIDList.push_back(UUID + L"=" + MetaName());
}

wstring v8MetaForm::getName()
{
    return Name;
}

wstring v8MetaForm::getUUID()
{
    return UUID;
}

v8MetaForm::v8MetaForm(v8Tree* t)
{
    UUID = L"00000000-0000-0000-0000-000000000000";
    Name = t->get_first()->get_first()->get_next()->get_first()->get_next()->get_first()->get_next()->get_first()->get_next()->get_next()->get_value();
}

v8MetaForm::~v8MetaForm()
{
}

