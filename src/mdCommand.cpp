#include "mdCommand.h"
#include "TStringList.h"
#include "StringUtils.h"
#include <stdexcept> // если нужно кидать исключения при отсутствии данных

// Реализация конструкторов
mdCommand::mdCommand(EctoTreeNode* Root)
{
    if (!Root)
    {
        ParserStr parser;

        std::string Caption = (*Root->children)[0]->caption;
        
        //auto result = parser.ParseString(Caption);
        
        //FUUID = result[1];
        FUUID = parser.ParseString(Caption)[1];
        
        auto children1 = (*Root->children)[1];
        
        auto children11 = (*children1->children)[1];
        auto children11_size = (*children11->children).size() - 1;

        FName = (*children1->children)[children11_size]->caption;

        //auto SL = parser.ParseString(FName);
        //FName = SL[2];
        FName = parser.ParseString(FName)[2];
    }
}


std::string mdCommand::MetaName()
{
    return "Команды." + FName;
}

std::string mdCommand::MetaClassName()
{
    return "Команды";
}

void mdCommand::FillUUID(THashedStringList UUIDList)
{
    UUIDList.Add(FUUID + "=" + MetaName());
}

/////////////////////////////////////////////////// mdFormList ////////////////////////
///////////////////////////////////////////////////////////////////////////////////////
mdCommand* mdCommandList::Get(int Index)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    return FList[Index].get();
}

void mdCommandList::Put(int Index, mdCommand* Item)
{
    if (Index < 0 || static_cast<size_t>(Index) >= FList.size())
        throw std::out_of_range("Index out of range");
    if (Item == nullptr)
        throw std::invalid_argument("Item cannot be null");

    FList[Index].reset(Item); // старый объект будет удалён автоматически
}

mdCommandList::mdCommandList(EctoTreeNode* Root, TCommandMode Mode)
{
    ParserStr parser;
    mdCommand mCommand = mdCommand(nullptr);

    auto SL = parser.ParseString(Root->caption);
    size_t value = std::stoi(SL[1]);
    for (size_t i = 0; i < value - 1; i++)
    {
        auto mCommandi    = (*Root        -> children)[i];
        auto mCommandi0   = (*mCommandi   -> children)[0];
        auto mCommandi00  = (*mCommandi0  -> children)[0];
        auto mCommandi000 = (*mCommandi00 -> children)[0];

        switch (Mode) {
        case TCommandMode::cmRef:
        case TCommandMode::cmBP:
            mCommand = mdCommand(mCommandi000);
            break;
        default:
            mCommand = mdCommand(mCommandi00);
        }
        FList.push_back(std::unique_ptr<mdCommand>(&mCommand));
    }

}


// Дополнительно (если нужно добавлять вручную)
void mdCommandList::Add(mdCommand* Item)
{
    if (!Item)
        throw std::invalid_argument("Item cannot be null");
    FList.push_back(std::unique_ptr<mdCommand>(Item));
}