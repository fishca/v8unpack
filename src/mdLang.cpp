#include "mdLang.h"
#include "TStringList.h"
#include "StringUtils.h"
#include "TMSTree.h"

#include <stdexcept> // если нужно кидать исключени€ при отсутствии данных

// –еализаци€ конструкторов
mdLang::mdLang(const std::string strUUID)
{


    //if (!Root)
    //{
    //    Parser parser;

    //    std::string Caption = (*Root->children)[0]->caption;
    //    
    //    auto result = parser.ParseString(Caption);
    //    
    //    FUUID = result[1];
    //    
    //    auto children1 = (*Root->children)[1];
    //    
    //    auto children11 = (*children1->children)[1];
    //    auto children11_size = (*children11->children).size() - 1;

    //    FName = (*children1->children)[children11_size]->caption;

    //    auto SL = parser.ParseString(FName);
    //    FName = SL[2];
    //}
}


std::string mdLang::MetaName()
{
    return "языки." + FName;
}

std::string mdLang::MetaClassName()
{
    return "языки";
}


/////////////////////////////////////////////////// mdFormList ////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
mdLang* mdLangList::Get(int Index)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    return FList[Index].get();
}

void mdLangList::Put(int Index, mdLang* Item)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    if (Item == nullptr)
        throw std::invalid_argument("Item cannot be null");

    FList[Index].reset(Item); // старый объект будет удалЄн автоматически
}

mdLangList::mdLangList(TMSTree MetaTree)
{
    ParserStr parser;
    //mdLang mCommand = mdLang(nullptr);

    //auto SL = parser.ParseString(Root->caption);
    //size_t value = std::stoi(SL[1]);
    //for (size_t i = 0; i < value - 1; i++)
    //{
    //    auto mCommandi    = (*Root        -> children)[i];
    //    auto mCommandi0   = (*mCommandi   -> children)[0];
    //    auto mCommandi00  = (*mCommandi0  -> children)[0];
    //    auto mCommandi000 = (*mCommandi00 -> children)[0];

    //    switch (Mode) {
    //    case TCommandMode::cmRef:
    //    case TCommandMode::cmBP:
    //        mCommand = mdLang(mCommandi000);
    //        break;
    //    default:
    //        mCommand = mdLang(mCommandi00);
    //    }
    //    FList.push_back(std::unique_ptr<mdLang>(&mCommand));
    //}

}


// ƒополнительно (если нужно добавл€ть вручную)
void mdLangList::Add(mdLang* Item)
{
    if (!Item)
        throw std::invalid_argument("Item cannot be null");
    FList.push_back(std::unique_ptr<mdLang>(Item));
}