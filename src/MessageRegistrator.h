#ifndef MessageRegistrationH
#define MessageRegistrationH

#include <list>
#include <string>
#include <iostream>


using namespace std;

enum MessageState
{
	msEmpty      = -1,
	msSuccesfull = 0,
	msWarning    = 1,
	msInfo       = 2,
	msError      = 3,
	msWait       = 4,
	msHint       = 5
};

typedef list<string> StringList;
typedef StringList::iterator StringListIter;

class MessageRegistrator
{
private:
	bool DebugMessage;

public:
	MessageRegistrator();

	void setDebugMode(bool dstate);
	bool getDebugMode();

	virtual void AddMessage(const string& description, const MessageState mstate, StringList* param = NULL) = 0;
	virtual void Status(const string& message) = 0;
	
	void AddError(const string& description);
	
	void AddError(const string& description, 
		const string& parname1, const string& par1);
	
	void AddError(const string& description,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2
	);
	
	void AddError(const string& description,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3
	);
	void AddError(const string& description,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4
	);
	void AddError(const string& description,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5
	);
	void AddError(const string& description,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5,
		const string& parname6, const string& par6
	);
	void AddError(const string& description,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5,
		const string& parname6, const string& par6,
		const string& parname7, const string& par7
	);

	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1
	);
	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2
	);
	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3
	);
	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4
	);
	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5
	);
	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5,
		const string& parname6, const string& par6
	);
	void AddMessage_(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5,
		const string& parname6, const string& par6,
		const string& parname7, const string& par7
	);

	void AddDebugMessage(const string& description, const MessageState mstate);

	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1
	);
	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2
	);
	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3
	);
	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4
	);

	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5
	);
	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5,
		const string& parname6, const string& par6
	);
	void AddDebugMessage(const string& description, const MessageState mstate,
		const string& parname1, const string& par1,
		const string& parname2, const string& par2,
		const string& parname3, const string& par3,
		const string& parname4, const string& par4,
		const string& parname5, const string& par5,
		const string& parname6, const string& par6,
		const string& parname7, const string& par7
	);
};
#endif
