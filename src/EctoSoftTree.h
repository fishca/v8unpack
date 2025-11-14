#ifndef ECTO_SOFT_TREE_H
#define ECTO_SOFT_TREE_H

#include <vector>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <cassert>

// Предварительное объявление классов
class EctoSoftTree;
class EctoTreeNode;

// Список узлов дерева
class EctoTreeList {
private:
    std::vector<EctoTreeNode*> nodes;

public:
    ~EctoTreeList();
    EctoTreeNode* get_(size_t index) const;
    void put(size_t index, EctoTreeNode* item);
    void add(EctoTreeNode* item);
    void insert(size_t index, EctoTreeNode* item);
    void remove(size_t index);
    int indexOf(EctoTreeNode* item) const;
    void deleteAt(size_t index);
    size_t count() const;
    size_t size() const;
    bool empty() const;
    EctoTreeNode* operator[](size_t index) const;
    EctoTreeNode*& operator[](size_t index);
};

// Узел дерева
class EctoTreeNode {
private:
    EctoTreeNode* parentNode;

public:
    EctoSoftTree* parentTree;
    std::unique_ptr<EctoTreeList> children;
    //std::vector<EctoTreeNode*> children;
    void* data;
    std::string caption;

    // Конструктор и деструктор
    EctoTreeNode();
    ~EctoTreeNode();

    // Геттер для parentNode
    EctoTreeNode* getParentNode() const { return parentNode; }

    // Сеттер для parentNode
    void setParentNode(EctoTreeNode* node);

    // Методы
    int getDescendantCount() const;
    int getAbsoluteIndex() const;
    int getChildIndex() const;
    int getLevel() const;
    EctoTreeNode* getPrevSibling() const;
    EctoTreeNode* getNextSibling() const;
    EctoTreeNode* getLastDescendant() const;
    void setParent(EctoTreeNode* newParentNode);
    EctoTreeNode* getPrevChild(EctoTreeNode* targetChildNode) const;
    EctoTreeNode* getNextChild(EctoTreeNode* targetChildNode) const;
    EctoTreeNode* getLastChild() const;
    EctoTreeNode* getNext();
    EctoTreeNode* getPrev();
    bool isRoot() const;
    bool isParentOf(EctoTreeNode* node) const;
    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();
    void sort(std::function<int(EctoTreeNode*, EctoTreeNode*)> compare, bool sortSubtrees);

    // Свойства
    int absoluteIndex() const { return getAbsoluteIndex(); }
    int index() const { return getChildIndex(); }
    EctoTreeNode* prevSibling() const { return getPrevSibling(); }
    EctoTreeNode* nextSibling() const { return getNextSibling(); }
    EctoTreeNode* lastDescendant() const { return getLastDescendant(); }
    int descendantCount() const { return getDescendantCount(); }
    int level() const { return getLevel(); }
};

// Дерево
class EctoSoftTree {
private:
    std::function<void(EctoTreeNode*)> onFreeNodeEvent;

    EctoTreeNode* getNodeFromIndex(int index);
    int getNodeCount() const;

public:
    EctoTreeNode* root;

    EctoSoftTree();
    ~EctoSoftTree();

    EctoTreeNode* findNode(const std::string& findCaption);
    void deleteNode(int index);
    void deleteNode(EctoTreeNode* deletingNode);
    EctoTreeNode* addNode(EctoTreeNode* parentNode);
    EctoTreeNode* addNode(EctoTreeNode* parentNode, const std::string& caption);
    EctoTreeNode* addNode(EctoTreeNode* parentNode, void* data);
    EctoTreeNode* addNode(EctoTreeNode* parentNode, const std::string& caption, void* data);
    void clear();

    // Свойства
    EctoTreeNode* nodes(int index) { return getNodeFromIndex(index); }
    int nodeCount() const { return getNodeCount(); }

    std::function<void(EctoTreeNode*)> getOnFreeNode() const { return onFreeNodeEvent; }
    void setOnFreeNode(const std::function<void(EctoTreeNode*)>& event) { onFreeNodeEvent = event; }
};

#endif //ECTO_SOFT_TREE_H
