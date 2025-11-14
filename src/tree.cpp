#include <sstream>
#include "tree.h"

TreeNode::~TreeNode()
{
    for (TreeNode* child : children) {
        delete child;
    }
}

void TreeNode::addChild(TreeNode* child)
{
    children.push_back(child);
}

void TreeNode::print(int level) const
{
    std::wcout << std::wstring(level * 2, ' ') << value << std::endl;
    for (const TreeNode* child : children) {
        child->print(level + 1);
    }
}
