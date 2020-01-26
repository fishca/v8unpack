#include "MessageRegistrator.h"

#include <list>
#include <string>
#include <iostream>

MessageRegistrator::MessageRegistrator()
{
	DebugMessage = false;
}

void MessageRegistrator::setDebugMode(bool dstate)
{
	DebugMessage = dstate;
}

bool MessageRegistrator::getDebugMode()
{
	return DebugMessage;
}

void MessageRegistrator::AddError(const string& description)
{
	AddMessage(description, msError);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	ts->push_back(parname5 + " = " + par5);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5,
	const string& parname6, const string& par6)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	ts->push_back(parname5 + " = " + par5);
	ts->push_back(parname6 + " = " + par6);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddError(const string& description,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5,
	const string& parname6, const string& par6,
	const string& parname7, const string& par7)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	ts->push_back(parname5 + " = " + par5);
	ts->push_back(parname6 + " = " + par6);
	ts->push_back(parname7 + " = " + par7);
	AddMessage(description, msError, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	ts->push_back(parname5 + " = " + par5);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5,
	const string& parname6, const string& par6)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	ts->push_back(parname5 + " = " + par5);
	ts->push_back(parname6 + " = " + par6);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddMessage_(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5,
	const string& parname6, const string& par6,
	const string& parname7, const string& par7)
{
	StringList* ts = new StringList;
	ts->push_back(parname1 + " = " + par1);
	ts->push_back(parname2 + " = " + par2);
	ts->push_back(parname3 + " = " + par3);
	ts->push_back(parname4 + " = " + par4);
	ts->push_back(parname5 + " = " + par5);
	ts->push_back(parname6 + " = " + par6);
	ts->push_back(parname7 + " = " + par7);
	AddMessage(description, mstate, ts);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate)
{
	if (!DebugMessage) 
		return;
	AddMessage(description, mstate);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1)
{
	if (!DebugMessage) 
		return;
	AddMessage_(description, mstate,
		parname1, par1);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2)
{
	if (!DebugMessage)
		return;
	AddMessage_(description, mstate,
		parname1, par1,
		parname2, par2);
}


void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3)
{
	if (!DebugMessage)
		return;
	AddMessage_(description, mstate,
		parname1, par1,
		parname2, par2,
		parname3, par3);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4)
{
	if (!DebugMessage)
		return;
	AddMessage_(description, mstate,
		parname1, par1,
		parname2, par2,
		parname3, par3,
		parname4, par4);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5)
{
	if (!DebugMessage)
		return;
	AddMessage_(description, mstate,
		parname1, par1,
		parname2, par2,
		parname3, par3,
		parname4, par4,
		parname5, par5);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5,
	const string& parname6, const string& par6)
{
	if (!DebugMessage)
		return;
	AddMessage_(description, mstate,
		parname1, par1,
		parname2, par2,
		parname3, par3,
		parname4, par4,
		parname5, par5,
		parname6, par6);
}

void MessageRegistrator::AddDebugMessage(const string& description, const MessageState mstate,
	const string& parname1, const string& par1,
	const string& parname2, const string& par2,
	const string& parname3, const string& par3,
	const string& parname4, const string& par4,
	const string& parname5, const string& par5,
	const string& parname6, const string& par6,
	const string& parname7, const string& par7)
{
	if (!DebugMessage)
		return;
	AddMessage_(description, mstate,
		parname1, par1,
		parname2, par2,
		parname3, par3,
		parname4, par4,
		parname5, par5,
		parname6, par6,
		parname7, par7);
}








