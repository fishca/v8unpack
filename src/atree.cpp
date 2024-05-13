/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/

#include "atree.h"
#include <algorithm>

template<class T>
inline int TEctoTreeList<T>::IndexOf(TEctoTreeNode* Item)
{
	int Result = 0;

	//while (Result < this->size() and this[Result] != Item)
	//{
	//	Result++;
	//}

	if (Result == this->size())
	{
		Result = -1;
	}

	return Result;
}


/*
template<class T>
int TEctoTreeList<T>::IndexOf(TEctoTreeNode* Item)
{
	int Result = 0;

	//while (Result < this->Items.size() and this[Result] != Item)
	while (Result < this->size() and this[Result] != Item)
	{
		Result++;
	}

	if (Result == this->size())
	{
		Result = -1;
	}

	return Result;
}

*/



int TEctoTreeNode::GetDescendantCount()
{
	TEctoTreeNode* Node;
	int Result = 0;

	Node = GetLastDescendant();
	if (!Node)
	{
		return 0;
	}

	while (Node != this)
	{
		Result++;
		Node = Node->GetPrev();
	}

	return Result;
}

int TEctoTreeNode::GetAbsoluteIndex()
{
	TEctoTreeNode* Node;
	int Result = 0;

	if (IsRoot())
	{
		Result = 0;
	}
	else
	{
		Result = -1;
		Node = this;
		while (Node != this)
		{
			Result++;
			Node = Node->GetPrev();
		}
	}

	return Result;
}

int TEctoTreeNode::IndexOf(TEctoTreeNode* Item, int vSize)
{

	return 0;
}

int TEctoTreeNode::GetChildIndex()
{
	int Result = -1;
	if (IsRoot())
		return Result;

	Result = ParentNode->Children.IndexOf(this);

	return Result;
}

int TEctoTreeNode::GetLevel()
{
	TEctoTreeNode* Node;
	int Result = 0;

	if (IsRoot())
	{
		return Result;
	}

	Node = this;

	while (Node != this)
	{
		Result++;
		Node = Node->ParentNode;

	}

	return Result;
}


TEctoTreeNode* TEctoTreeNode::GetPrevSibling()
{
	if (IsRoot())
		return nullptr;
	else
	{
		return ParentNode->GetPrevChild(this);
	}
}

TEctoTreeNode* TEctoTreeNode::GetNextSibling()
{
	if (IsRoot())
		return nullptr;
	else
	{
		return ParentNode->GetNextChild(this);
	}
}

TEctoTreeNode* TEctoTreeNode::GetLastDescendant()
{

	TEctoTreeNode* Node = this;

	while (Node->GetLastChild() != nullptr)
	{
		Node = Node->GetLastChild();
	}

	if (Node == this)
	{
		Node = nullptr;
	}

	return Node;
}

void TEctoTreeNode::SetParent(TEctoTreeNode* NewParentNode)
{
	if ((NewParentNode == nullptr) or (NewParentNode == this))
		return;

	ParentNode->Children.erase(Children.begin() + Children.IndexOf(this));
	//NewParentNode->Children.push_back(this);
	this->ParentNode = NewParentNode;

}


TEctoTreeNode* TEctoTreeNode::GetPrevChild(TEctoTreeNode* TargetChildNode)
{
	TEctoTreeNode* Result = nullptr;

	int PrevChildIndex = Children.IndexOf(TargetChildNode) - 1;
	//if (PrevChildIndex > -1)
	//	Result = Children[PrevChildIndex];

	return Result;
}

TEctoTreeNode* TEctoTreeNode::GetNextChild(TEctoTreeNode* TargetChildNode)
{
	TEctoTreeNode* Result = nullptr;

	int NextChildIndex = Children.IndexOf(TargetChildNode) + 1;
	//if (NextChildIndex < Children.size() and NextChildIndex > 0)
	//	Result = Children[NextChildIndex];

	return Result;
}

TEctoTreeNode* TEctoTreeNode::GetLastChild()
{
	TEctoTreeNode* Result = nullptr;

	//if (Children.size() > 0)
	//	Result = Children[Children.size() - 1];

	return Result;
}

TEctoTreeNode* TEctoTreeNode::GetNext()
{
	TEctoTreeNode* Result = nullptr;
	TEctoTreeNode* Node = nullptr;

//	if (Children.size() > 0)
//		Result = Children[0];

	if (Result == nullptr)
		Result = GetNextSibling();

	if (Result == nullptr and not IsRoot())
	{
		Node = ParentNode;
		while (Node->GetNextSibling() == nullptr and not Node->IsRoot())
		{
			Node = Node->ParentNode;
		}
		if (not Node->IsRoot())
			Result = Node->GetNextSibling();
	}

	return Result;
}

TEctoTreeNode* TEctoTreeNode::GetPrev()
{
	TEctoTreeNode* Result = nullptr;
	TEctoTreeNode* Node = nullptr;

	if (IsRoot())
		return Result;

	Result = GetPrevSibling();

	if (Result == nullptr)
	{
		Result = ParentNode;
	}
	else
	{
		Node = Result->LastDescendant;
		if (Node != nullptr)
			Result = Node;
	}
	return Result;
}

bool TEctoTreeNode::IsRoot()
{
	return (this == ParentTree->Root);
}

bool TEctoTreeNode::IsParentOf(TEctoTreeNode* Node)
{
	TEctoTreeNode* TempNode = nullptr;

	bool Result = false;

	TempNode = Node->ParentNode;
	while (TempNode != nullptr)
	{
		if (TempNode == this)
		{
			Result = true;
			break;
		}
		TempNode = TempNode->ParentNode;
	}

	return Result;
}

void TEctoTreeNode::MoveUP()
{
}

void TEctoTreeNode::MoveDown()
{
}

void TEctoTreeNode::MoveLeft()
{
}

void TEctoTreeNode::MoveRight()
{
}


////////////////////////////////////////////////////////
TEctoTreeNode* TEctoSoftTree::FindNode(std::string FindCaption)
{
	return nullptr;
}

void TEctoSoftTree::DeleteNode(int Index)
{
}

void TEctoSoftTree::DeleteNode(TEctoTreeNode* DeletingNode)
{
}

TEctoTreeNode* TEctoSoftTree::AddNode(TEctoTreeNode* aParentNode)
{
	return nullptr;
}

TEctoTreeNode* TEctoSoftTree::AddNode(TEctoTreeNode* aParentNode, int Data)
{
	return nullptr;
}

TEctoTreeNode* TEctoSoftTree::AddNode(TEctoTreeNode* aParentNode, std::string Caption)
{
	return nullptr;
}

TEctoTreeNode* TEctoSoftTree::AddNode(TEctoTreeNode* aParentNode, std::string Caption, int Data)
{
	return nullptr;
}

void TEctoSoftTree::Clear()
{
}

TEctoTreeNode* TEctoSoftTree::GetNodeFromIndex(int Index)
{
	return nullptr;
}

int TEctoSoftTree::GetNodeCount()
{
	return 0;
}

