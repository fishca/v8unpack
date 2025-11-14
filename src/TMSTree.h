#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_set>

#include "EctoSoftTree.h"

enum class TMetaParserStates {
    mpsIdle, 
    mpsReadInfoField, 
    mpsReadNumField, 
    mpsReadStrField,
    mpsReadQuotedField, 
    mpsReadBase64,
    mpsReadQuotedSubStr, 
    mpsFoundQuote,
    mpsExitQuotes, 
    mpsWaitForQuote, 
    mpsError
};


class TMSTree : public EctoSoftTree
{
private:
    std::string FPData;
    void ParseMetaDataStream(std::string aMetaDataStream, bool addLFCR = false);
    void ParseBaseDataStream(std::string aMetaDataStream);
    void ParseBaseDataStream2(std::string aMetaDataStream);
    void ParseLevel(std::string aMetaDataStream, int index, EctoTreeNode* TreeParent, bool addLFCR = false);
public:
    void LoadTree(std::string PData, bool Mode, bool addLFCR = false);
    void UploadToFile(std::string FileName);
    void UploadToString(std::string OutData);
    EctoTreeNode* GetItemByName(std::string ItemName);
    void TranslateTree();
    void TranslateForm();

};
