#include "StringUtils.h"
#include "mdObject.h"
#include "TMStree.h"




mdObject::mdObject(const std::string& strUUID, const std::string& FormsUUID)
{
	TMSTree ST;
	ParserStr parser;

	ST.LoadTree(strUUID, true);
	
	auto root = ST.nodes(0);
	auto child0 = (*root->children)[0];
	auto child00 = (*child0->children)[0];
	auto child000 = (*child00->children)[0];
	auto child0000 = (*child00->children)[0];
	
	FName = child0000->caption;

	//auto SL = parser.ParseString(FName);
	
	//FName = SL[2];
	FName = parser.ParseString(FName)[2];

	// Определяем наличие формы
	if (FormsUUID != "")
	{
		auto countChild = (*child00->children).size() - 1;
		for (size_t i = 0; i < countChild; i++)
		{
			std::string captionForm = (*child0->children)[i]->caption;
			if (SameText(FormsUUID, captionForm)) {
				//FForms = mdForm();
			}

		}
	}
}


mdObject::~mdObject()
{
}

void mdObject::FillUUID(THashedStringList* UUIDList, THashedStringList* UUIDClassList, THashedStringList* FormsList, THashedStringList* MoxelsList)
{
}
