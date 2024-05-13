/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/
#pragma once

#include <string>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

class TEctoTreeNode;
class TEctoSoftTree;

//------------------------------------------------------------------------------
// Список узлов дерева
//------------------------------------------------------------------------------


template<class T>
class TEctoTreeList : public std::vector<T*>
{
public:

	int IndexOf(TEctoTreeNode* Item);

	TEctoTreeList()
	{
	}

	~TEctoTreeList()
	{
	}

private:
	//std::vector<T*> Items;

};



//------------------------------------------------------------------------------
// Узел дерева
//------------------------------------------------------------------------------
class TEctoTreeNode
{
public:
	TEctoSoftTree* ParentTree;
	//std::vector<TEctoTreeNode*> Children;

	TEctoTreeList<TEctoTreeNode*> Children;

	int Data; // Data: Pointer;
	std::string Caption;

	TEctoTreeNode* GetPrevChild(TEctoTreeNode* TargetChildNode); // получить предыдущего ребенка
	TEctoTreeNode* GetNextChild(TEctoTreeNode* TargetChildNode); // получить следующего ребенка

	TEctoTreeNode* GetLastChild(); // получить последнего ребенка
	TEctoTreeNode* GetNext();      // получить следующего 
	TEctoTreeNode* GetPrev();      // получить предыдущего  

	bool IsRoot();                         // это корень
	bool IsParentOf(TEctoTreeNode* Node);  // это родитель

	int IndexOf(TEctoTreeNode* Item, int vSize);

	void MoveUP();
	void MoveDown();
	void MoveLeft();
	void MoveRight();

	TEctoTreeNode()
	{
	}

	~TEctoTreeNode()
	{
	}

private:
	TEctoTreeNode* FParentNode;
	TEctoTreeNode* PrevSibling;
	TEctoTreeNode* NextSibling;
	TEctoTreeNode* LastDescendant;
	TEctoTreeNode* ParentNode;

	int AbsoluteIndex;
	int Index;
	int DescendantCount;
	int Level;

	int GetDescendantCount(); // количество потомков
	int GetAbsoluteIndex();   // абсолютный индекс
	int GetChildIndex();      // индекс "ребенка"
	int GetLevel();           // получить уровень

	TEctoTreeNode* GetPrevSibling();                 // получить предыдущего брата/сестру
	TEctoTreeNode* GetNextSibling();                 // получить следующего брата/сестру
	TEctoTreeNode* GetLastDescendant();              // получить последнего потомка   
	void SetParent(TEctoTreeNode* NewParentNode);    // установить родителя

};

//------------------------------------------------------------------------------
// Дерево
//------------------------------------------------------------------------------
class TEctoSoftTree
{
public:
	TEctoTreeNode* Root;

	TEctoTreeNode* FindNode(std::string FindCaption);
	void DeleteNode(int Index);
	void DeleteNode(TEctoTreeNode* DeletingNode);

	TEctoTreeNode* AddNode(TEctoTreeNode* aParentNode);
	TEctoTreeNode* AddNode(TEctoTreeNode* aParentNode, int Data);
	TEctoTreeNode* AddNode(TEctoTreeNode* aParentNode, std::string Caption);
	TEctoTreeNode* AddNode(TEctoTreeNode* aParentNode, std::string Caption, int Data);
	
	void Clear();

	TEctoSoftTree()
	{
	}

	~TEctoSoftTree()
	{
	}

private:
	
	std::vector<TEctoTreeNode*> Nodes;
	int NodeCount;

	TEctoTreeNode* GetNodeFromIndex(int Index);
	int GetNodeCount();
	

};

