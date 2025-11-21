#include "TMSTree.h"
#include "EctoSoftTree.h"
#include <string>
#include <algorithm>
#include <cctype>

void TMSTree::ParseMetaDataStream(std::string aMetaDataStream, bool addLFCR)
{
	size_t i = 0;
	ParseLevel(aMetaDataStream, i, addNode(NULL, "ROOT"), addLFCR);
	i++;
	while (i < aMetaDataStream.size())
	{
		ParseLevel(aMetaDataStream, i, nodes(0), addLFCR);
		i++;
	}
}

void TMSTree::ParseBaseDataStream(std::string aMetaDataStream)
{
	std::string s;

	// Remove CR and LF characters
	for (char c : aMetaDataStream) {
		if (c != '\n' && c != '\r') {
			s += c;
		}
	}

	clear();
	EctoTreeNode* CurrNode = nullptr;

	size_t i = 0;
	while (i < s.length()) {
		char c = s[i];
		size_t p = 1;

		if (c == '{') {
			// Find next '}' or '{'
			size_t foundPos = std::string::npos;
			for (size_t j = i + 1; j < s.length(); j++) {
				if (s[j] == '}' || s[j] == '{') {
					foundPos = j;
					break;
				}
			}

			if (foundPos != std::string::npos) {
				p = foundPos - i;
				std::string Item = s.substr(i + 1, p - 1);

				if (!Item.empty() && Item.back() == ',') {
					Item.pop_back();
				}

				if (CurrNode) {
					CurrNode = addNode(CurrNode, Item);
				}
				else {
					CurrNode = addNode(nullptr, Item);
				}
			}
		}
		else if (c == '}') {
			if (CurrNode) {
				CurrNode = CurrNode->getParentNode();
			}
		}

		i += p;
	}
}

void TMSTree::ParseBaseDataStream2(std::string aMetaDataStream)
{
	// Remove CR and LF characters using remove_if
	std::string s;
	s.reserve(aMetaDataStream.length());
	std::copy_if(aMetaDataStream.begin(), aMetaDataStream.end(),
		std::back_inserter(s),
		[](char c) { return c != '\n' && c != '\r'; });

	clear();
	EctoTreeNode* CurrNode = NULL;

	size_t i = 0;
	while (i < s.length()) {
		char c = s[i];
		size_t p = 1;

		if (c == '{') {
			// Find position of next '}' or '{'
			size_t foundPos = s.find_first_of("{}", i + 1);

			if (foundPos != std::string::npos) {
				p = foundPos - i;
				std::string Item = s.substr(i + 1, p - 1);

				// Remove trailing comma if present
				if (!Item.empty() && Item[Item.length() - 1] == ',') {
					Item.erase(Item.length() - 1);
				}

				CurrNode = addNode(CurrNode, Item);
			}
		}
		else if (c == '}') {
			if (CurrNode) {
				CurrNode = CurrNode->getParentNode();
			}
		}

		i += p;
	}
}

void TMSTree::ParseLevel(std::string aMetaDataStream, size_t index, EctoTreeNode* TreeParent, bool addLFCR)
{
	char CurChar = ' ';
	std::string FieldStr = "";
	EctoTreeNode *CurNode;
	TMetaParserStates ParserState = TMetaParserStates::mpsIdle;

	const char OPEN_BRACE    = '{';
	const char CLOSE_BRACE   = '}';
	const char QUESTION_MARK = '?'; // #$3F
	const char CR = '\r';           // #$0D
	const char LF = '\n';           // #$0A

	CurNode = addNode(TreeParent, FieldStr);
	while (index < aMetaDataStream.size())
	{
		CurChar = aMetaDataStream[index];

		switch (CurChar)
		{
			case OPEN_BRACE:
				
				if (ParserState == TMetaParserStates::mpsIdle) {
					ParserState = TMetaParserStates::mpsReadInfoField;
				}
				else if (ParserState == TMetaParserStates::mpsReadInfoField) {
					ParseLevel(aMetaDataStream, index, CurNode);
				}
				else {
					ParserState = TMetaParserStates::mpsError;
				}
				
				break;			

			case CLOSE_BRACE:
				
				if (ParserState == TMetaParserStates::mpsReadInfoField) {
					CurNode->caption = FieldStr;
					return;
				}
				else {
					ParserState = TMetaParserStates::mpsError;
				}
				break;

			case QUESTION_MARK:
			
				FieldStr += ' ';
				break;
			
			case CR:
			case LF:
			
				if (addLFCR) {
					FieldStr += CurChar;
				}
				break;

			default:
				
				if (CurChar > CR) {
					FieldStr += CurChar;
				}
				break;

		}
		if (ParserState == TMetaParserStates::mpsError)
		{

		}
		index++;
	}
}

void TMSTree::LoadTree(std::string PData, bool Mode, bool addLFCR)
{
	if (PData.empty()) {
		return;
	}

	FPData = PData;

	if (Mode) {
		ParseMetaDataStream(FPData, addLFCR);
	}
	else {
		ParseBaseDataStream(FPData);
	}
}

void TMSTree::UploadToFile(std::string FileName)
{
	//TODO: Реализовать void TMSTree::UploadToFile(std::string FileName)
}

void TMSTree::UploadToString(std::string OutData)
{
	OutData = "Пока не реализовано!";
	//TODO: Реализовать void TMSTree::UploadToString(std::string OutData)
}

EctoTreeNode* TMSTree::GetItemByName(std::string ItemName)
{
	for (int i = 0; i < nodeCount() - 1; i++)
	{
		if (nodes(i)->caption.empty())
			continue;

		if (nodes(i)->caption.substr(0, ItemName.length()) == ItemName) {
			return nodes(i);

		}
	}
	return nullptr;
}

void TMSTree::TranslateTree()
{
	//TODO: Реализовать void TMSTree::TranslateTree()
}

void TMSTree::TranslateForm()
{
	//TODO: Реализовать void TMSTree::TranslateForm()
}
