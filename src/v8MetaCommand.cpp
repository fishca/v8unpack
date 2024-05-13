#include "v8MetaCommand.h"

wstring v8MetaCommand::MetaName()
{
	return L"Команды." + Name;
}

wstring v8MetaCommand::MetaClassName()
{
	return L"Команды";
}

void v8MetaCommand::FillUUID(list<wstring> UUIDList)
{
	UUIDList.push_back(UUID + L"=" + MetaName());
}

v8MetaCommand::v8MetaCommand(v8Tree* t)
{
	UUID = t->get_first()->get_first()->get_next()->get_first()->get_next()->get_first()->get_next()->get_first()->get_next()->get_value();
	Name = t->get_first()->get_first()->get_next()->get_first()->get_next()->get_first()->get_next()->get_next()->get_first()->get_next()->get_next()->get_next()->get_next()->get_next()->get_next()->get_next()->get_next()->get_next()->get_first()->get_next()->get_next()->get_value();
}

v8MetaCommand::~v8MetaCommand()
{
}

wstring v8MetaCommand::getName()
{
	return Name;
}

wstring v8MetaCommand::getUUID()
{
	return UUID;
}