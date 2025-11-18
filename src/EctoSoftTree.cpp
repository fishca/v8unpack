#include "EctoSoftTree.h"

// Реализация EctoTreeList
EctoTreeList::~EctoTreeList() {
    // Очистка памяти будет выполняться в EctoTreeNode
}

EctoTreeNode* EctoTreeList::get_(size_t index) const {
    if (index >= nodes.size()) {
        throw std::out_of_range("Index out of range");
    }
    return nodes[index];
}

void EctoTreeList::put(size_t index, EctoTreeNode* item) {
    if (index >= nodes.size()) {
        throw std::out_of_range("Index out of range");
    }
    nodes[index] = item;
}

void EctoTreeList::add(EctoTreeNode* item) {
    nodes.push_back(item);
}

void EctoTreeList::insert(size_t index, EctoTreeNode* item) {
    if (index > nodes.size()) {
        throw std::out_of_range("Index out of range");
    }
    nodes.insert(nodes.begin() + index, item);
}

void EctoTreeList::remove(size_t index) {
    if (index >= nodes.size()) {
        throw std::out_of_range("Index out of range");
    }
    nodes.erase(nodes.begin() + index);
}

int EctoTreeList::indexOf(EctoTreeNode* item) const {
    auto it = std::find(nodes.begin(), nodes.end(), item);
    if (it != nodes.end()) {
        return static_cast<int>(std::distance(nodes.begin(), it));
    }
    return -1;
}

void EctoTreeList::deleteAt(size_t index) {
    if (index >= nodes.size()) {
        throw std::out_of_range("Index out of range");
    }
    nodes.erase(nodes.begin() + index);
}

size_t EctoTreeList::count() const {
    return nodes.size();
}

size_t EctoTreeList::size() const {
    return nodes.size();
}

bool EctoTreeList::empty() const {
    return nodes.empty();
}

EctoTreeNode* EctoTreeList::operator[](size_t index) const {
    return get_(index);
}

EctoTreeNode*& EctoTreeList::operator[](size_t index) {
    if (index >= nodes.size()) {
        throw std::out_of_range("Index out of range");
    }
    return nodes[index];
}

// Реализация EctoTreeNode
EctoTreeNode::EctoTreeNode() : parentNode(nullptr), parentTree(nullptr), data(nullptr) {
    children = std::make_unique<EctoTreeList>();
}

EctoTreeNode::~EctoTreeNode() {
    // Рекурсивное удаление всех дочерних узлов
    for (size_t i = 0; i < children->count(); i++) {
        delete (*children)[i];
    }
}

void EctoTreeNode::setParentNode(EctoTreeNode* node) {
    parentNode = node;
    if (node != nullptr) {
        parentTree = node->parentTree;
    }
}

int EctoTreeNode::getDescendantCount() const {
    int count = 0;
    EctoTreeNode* node = getLastDescendant();
    if (node == nullptr) return 0;

    EctoTreeNode* current = node;
    while (current != this && current != nullptr) {
        count++;
        current = current->getPrev();
        if (current == node) break; // Защита от бесконечного цикла
    }
    return count;
}

int EctoTreeNode::getAbsoluteIndex() const {
    if (isRoot()) {
        return 0;
    }

    int result = 0;
    EctoTreeNode* node = parentTree->root;

    std::function<bool(EctoTreeNode*)> traverse;
    traverse = [&](EctoTreeNode* current) -> bool {
        if (current == this) {
            return true;
        }
        result++;

        for (size_t i = 0; i < current->children->count(); i++) {
            if (traverse((*current->children)[i])) {
                return true;
            }
        }
        return false;
        };

    traverse(node);
    return result;
}

int EctoTreeNode::getChildIndex() const {
    if (isRoot()) return -1;
    return getParentNode()->children->indexOf(const_cast<EctoTreeNode*>(this));
}

int EctoTreeNode::getLevel() const {
    if (isRoot()) return 0;

    int result = 0;
    EctoTreeNode* node = const_cast<EctoTreeNode*>(this);
    while (node != parentTree->root && node != nullptr) {
        result++;
        node = node->getParentNode();
    }
    return result;
}

EctoTreeNode* EctoTreeNode::getPrevSibling() const {
    if (isRoot()) return nullptr;
    return getParentNode()->getPrevChild(const_cast<EctoTreeNode*>(this));
}

EctoTreeNode* EctoTreeNode::getNextSibling() const {
    if (isRoot()) return nullptr;
    return getParentNode()->getNextChild(const_cast<EctoTreeNode*>(this));
}

EctoTreeNode* EctoTreeNode::getLastDescendant() const {
    EctoTreeNode* node = const_cast<EctoTreeNode*>(this);
    while (node->getLastChild() != nullptr) {
        node = node->getLastChild();
    }
    return (node == this) ? nullptr : node;
}

void EctoTreeNode::setParent(EctoTreeNode* newParentNode) {
    if (newParentNode == nullptr || newParentNode == this) return;

    if (getParentNode() != nullptr) {
        int index = getParentNode()->children->indexOf(this);
        if (index != -1) {
            getParentNode()->children->remove(index);
        }
    }

    newParentNode->children->add(this);
    setParentNode(newParentNode);
}

EctoTreeNode* EctoTreeNode::getPrevChild(EctoTreeNode* targetChildNode) const {
    int prevChildIndex = children->indexOf(targetChildNode) - 1;
    if (prevChildIndex >= 0) {
        return (*children)[prevChildIndex];
    }
    return nullptr;
}

EctoTreeNode* EctoTreeNode::getNextChild(EctoTreeNode* targetChildNode) const {
    int nextChildIndex = children->indexOf(targetChildNode) + 1;
    if (nextChildIndex < static_cast<int>(children->count()) && nextChildIndex >= 0) {
        return (*children)[nextChildIndex];
    }
    return nullptr;
}

EctoTreeNode* EctoTreeNode::getLastChild() const {
    if (children->empty()) return nullptr;
    return (*children)[children->count() - 1];
}

EctoTreeNode* EctoTreeNode::getNext() {
    // Если есть дочерние узлы, следующий - первый дочерний
    if (!children->empty()) {
        return (*children)[0];
    }

    // Если дочерних нет, следующий - следующий сестринский узел
    EctoTreeNode* result = getNextSibling();
    if (result != nullptr) return result;

    // Если и сестринских нет, ищем первого сестринского у родителя
    if (!isRoot()) {
        EctoTreeNode* node = getParentNode();
        while (node != nullptr && node->getNextSibling() == nullptr && !node->isRoot()) {
            node = node->getParentNode();
        }
        if (node != nullptr && !node->isRoot()) {
            result = node->getNextSibling();
        }
    }

    return result;
}

EctoTreeNode* EctoTreeNode::getPrev() {
    if (isRoot()) return nullptr;

    EctoTreeNode* result = getPrevSibling();
    if (result == nullptr) {
        result = getParentNode();
    }
    else {
        EctoTreeNode* node = result->getLastDescendant();
        if (node != nullptr) {
            result = node;
        }
    }

    return result;
}

bool EctoTreeNode::isRoot() const {
    return (parentTree != nullptr && this == parentTree->root);
}

bool EctoTreeNode::isParentOf(EctoTreeNode* node) const {
    if (node == nullptr) return false;

    EctoTreeNode* tempNode = node->getParentNode();
    while (tempNode != nullptr) {
        if (tempNode == this) {
            return true;
        }
        tempNode = tempNode->getParentNode();
    }
    return false;
}

void EctoTreeNode::moveUp() {
    if (isRoot()) return;

    EctoTreeNode* prevSibling = getPrevSibling();
    if (prevSibling != nullptr) {
        int childIndex = getChildIndex();
        EctoTreeNode* temp = (*getParentNode()->children)[childIndex];
        (*getParentNode()->children)[childIndex] = (*getParentNode()->children)[childIndex - 1];
        (*getParentNode()->children)[childIndex - 1] = temp;
    }
    else {
        if (!getParentNode()->isRoot()) {
            setParent(getParentNode()->getParentNode());
            moveUp();
        }
    }
}

void EctoTreeNode::moveDown() {
    if (isRoot()) return;
    EctoTreeNode* nextSibling = getNextSibling();
    if (nextSibling != nullptr) {
        int childIndex = getChildIndex();
        EctoTreeNode* temp = (*getParentNode()->children)[childIndex];
        (*getParentNode()->children)[childIndex] = (*getParentNode()->children)[childIndex + 1];
        (*getParentNode()->children)[childIndex + 1] = temp;
    }
}

void EctoTreeNode::moveLeft() {
    if (getParentNode() == nullptr || getParentNode()->isRoot() || isRoot()) return;
    if (getParentNode()->getParentNode() == nullptr) return;

    getParentNode()->getParentNode()->children->insert(getParentNode()->getChildIndex() + 1, this);

    int indexInParent = getParentNode()->children->indexOf(this);
    if (indexInParent != -1) {
        getParentNode()->children->remove(indexInParent);
    }

    setParentNode(getParentNode()->getParentNode());
}

void EctoTreeNode::moveRight() {
    if (isRoot() || getPrevSibling() == nullptr) return;
    setParent(getPrevSibling());
}

void EctoTreeNode::sort(std::function<int(EctoTreeNode*, EctoTreeNode*)> compare, bool sortSubtrees) {
    // Простая пузырьковая сортировка
    for (size_t j = 0; j < children->count(); j++) {
        for (size_t i = children->count() - 1; i > j; i--) {
            if (compare((*children)[i], (*children)[i - 1]) < 0) {
                std::swap((*children)[i], (*children)[i - 1]);
            }
        }

        if (sortSubtrees) {
            (*children)[j]->sort(compare, true);
        }
    }
}

// Реализация EctoSoftTree
EctoSoftTree::EctoSoftTree() : root(nullptr) {}

EctoSoftTree::~EctoSoftTree() {
    clear();
}

EctoTreeNode* EctoSoftTree::findNode(const std::string& findCaption) {
    if (root == nullptr) return nullptr;

    EctoTreeNode* result = nullptr;

    std::function<bool(EctoTreeNode*)> findNodeImpl;
    findNodeImpl = [&](EctoTreeNode* targetNode) -> bool {
        if (result != nullptr) return true;

        if (targetNode->caption == findCaption) {
            result = targetNode;
            return true;
        }

        for (size_t i = 0; i < targetNode->children->count(); i++) {
            if (findNodeImpl((*targetNode->children)[i])) {
                return true;
            }
        }
        return false;
        };

    findNodeImpl(root);
    return result;
}

void EctoSoftTree::deleteNode(int index) {
    deleteNode(getNodeFromIndex(index));
}

void EctoSoftTree::deleteNode(EctoTreeNode* deletingNode) {
    if (deletingNode == nullptr) return;

    // Вызываем callback перед удалением, если он установлен
    if (onFreeNodeEvent) {
        onFreeNodeEvent(deletingNode);
    }

    if (deletingNode->isRoot()) {
        // Для корневого узла просто удаляем его
        delete root;
        root = nullptr;
    }
    else {
        // Для не-корневого узла сначала удаляем из списка детей родителя
        EctoTreeNode* parent = deletingNode->getParentNode();
        if (parent != nullptr) {
            int index = parent->children->indexOf(deletingNode);
            if (index != -1) {
                parent->children->remove(index);
            }
        }
        // Затем удаляем сам узел
        delete deletingNode;
    }
}

EctoTreeNode* EctoSoftTree::addNode(EctoTreeNode* parentNode, const std::string& caption, void* data) {
    EctoTreeNode* newNode = new EctoTreeNode();

    if (root == nullptr) {
        newNode->setParentNode(nullptr);
        root = newNode;
        newNode->parentTree = this;
    }
    else {
        if (parentNode == nullptr) {
            delete newNode;
            throw std::invalid_argument("Parent node must exist");
        }
        newNode->setParentNode(parentNode);
        newNode->parentTree = this;
        parentNode->children->add(newNode);
    }

    newNode->caption = caption;
    newNode->data = data;

    return newNode;
}

EctoTreeNode* EctoSoftTree::addNode(EctoTreeNode* parentNode) {
    return addNode(parentNode, "", nullptr);
}

EctoTreeNode* EctoSoftTree::addNode(EctoTreeNode* parentNode, const std::string& caption) {
    return addNode(parentNode, caption, nullptr);
}

EctoTreeNode* EctoSoftTree::addNode(EctoTreeNode* parentNode, void* data) {
    return addNode(parentNode, "", data);
}

void EctoSoftTree::clear() {
    if (root == nullptr) return;

    // Вызываем callback для всех узлов перед очисткой
    if (onFreeNodeEvent) {
        std::function<void(EctoTreeNode*)> traverse;
        traverse = [&](EctoTreeNode* node) {
            onFreeNodeEvent(node);
            for (size_t i = 0; i < node->children->count(); i++) {
                traverse((*node->children)[i]);
            }
            };
        traverse(root);
    }

    delete root;
    root = nullptr;
}

EctoTreeNode* EctoSoftTree::getNodeFromIndex(int index) {
    if (root == nullptr) {
        throw std::invalid_argument("Tree is empty");
    }

    int indexCounter = -1;
    EctoTreeNode* result = nullptr;

    std::function<bool(EctoTreeNode*)> traverse;
    traverse = [&](EctoTreeNode* node) -> bool {
        indexCounter++;
        if (indexCounter == index) {
            result = node;
            return true;
        }

        for (size_t i = 0; i < node->children->count(); i++) {
            if (traverse((*node->children)[i])) {
                return true;
            }
        }
        return false;
        };

    if (traverse(root)) {
        return result;
    }

    throw std::invalid_argument("Wrong index");
}

int EctoSoftTree::getNodeCount() const {
    if (root == nullptr) return 0;
    return root->getDescendantCount() + 1; // +1 для корня
}

// Тесты (остаются без изменений)
class TreeTests {
public:
    static void runAllTests() {
        testBasicTreeCreation();
        testNodeProperties();
        testNavigation();
        testMovement();
        testSearch();
        testDeletion();
        testSorting();
        testOnFreeNodeCallback();
        std::cout << "All tests passed!" << std::endl;
    }

private:
    static void testBasicTreeCreation() {
        std::cout << "Testing basic tree creation..." << std::endl;

        EctoSoftTree tree;
        assert(tree.root == nullptr);
        assert(tree.nodeCount() == 0);

        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        assert(root != nullptr);
        assert(tree.root == root);
        assert(tree.nodeCount() == 1);
        assert(root->isRoot());
        assert(root->level() == 0);
        assert(root->index() == -1);

        EctoTreeNode* child1 = tree.addNode(root, "Child1");
        EctoTreeNode* child2 = tree.addNode(root, "Child2");

        assert(tree.nodeCount() == 3);
        assert(child1->level() == 1);
        assert(child2->level() == 1);
        assert(child1->index() == 0);
        assert(child2->index() == 1);

        std::cout << "Basic tree creation test passed!" << std::endl;
    }

    static void testNodeProperties() {
        std::cout << "Testing node properties..." << std::endl;

        EctoSoftTree tree;
        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        EctoTreeNode* child1 = tree.addNode(root, "Child1");
        EctoTreeNode* child2 = tree.addNode(root, "Child2");
        EctoTreeNode* grandchild = tree.addNode(child1, "Grandchild");

        // Test parent relationships
        assert(child1->getParentNode() == root);
        assert(child2->getParentNode() == root);
        assert(grandchild->getParentNode() == child1);

        // Test sibling relationships
        assert(child1->nextSibling() == child2);
        assert(child2->prevSibling() == child1);
        assert(child1->prevSibling() == nullptr);
        assert(child2->nextSibling() == nullptr);

        // Test descendant count
        assert(root->descendantCount() == 3);
        assert(child1->descendantCount() == 1);
        assert(child2->descendantCount() == 0);
        assert(grandchild->descendantCount() == 0);

        std::cout << "Node properties test passed!" << std::endl;
    }

    static void testNavigation() {
        std::cout << "Testing navigation..." << std::endl;

        EctoSoftTree tree;
        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        EctoTreeNode* child1 = tree.addNode(root, "Child1");
        EctoTreeNode* child2 = tree.addNode(root, "Child2");
        EctoTreeNode* grandchild = tree.addNode(child1, "Grandchild");

        // Test getNext()
        assert(root->getNext() == child1);
        assert(child1->getNext() == grandchild);
        assert(grandchild->getNext() == child2);
        assert(child2->getNext() == nullptr);

        // Test getPrev()
        assert(child2->getPrev() == grandchild);
        assert(grandchild->getPrev() == child1);
        assert(child1->getPrev() == root);
        assert(root->getPrev() == nullptr);

        std::cout << "Navigation test passed!" << std::endl;
    }

    static void testMovement() {
        std::cout << "Testing movement..." << std::endl;

        EctoSoftTree tree;
        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        tree.addNode(root, "Child1"); // unused in test
        EctoTreeNode* child2 = tree.addNode(root, "Child2");
        tree.addNode(root, "Child3"); // unused in test

        // Test moveUp/moveDown
        assert(child2->index() == 1);
        child2->moveUp();
        assert(child2->index() == 0);
        child2->moveDown();
        assert(child2->index() == 1);

        std::cout << "Movement test passed!" << std::endl;
    }

    static void testSearch() {
        std::cout << "Testing search..." << std::endl;

        EctoSoftTree tree;
        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        EctoTreeNode* child1 = tree.addNode(root, "Child1");
        tree.addNode(root, "Child2"); // unused in test
        EctoTreeNode* grandchild = tree.addNode(child1, "Grandchild");

        EctoTreeNode* found = tree.findNode("Child1");
        assert(found == child1);

        found = tree.findNode("Grandchild");
        assert(found == grandchild);

        found = tree.findNode("NonExistent");
        assert(found == nullptr);

        std::cout << "Search test passed!" << std::endl;
    }

    static void testDeletion() {
        std::cout << "Testing deletion..." << std::endl;

        EctoSoftTree tree;
        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        EctoTreeNode* child1 = tree.addNode(root, "Child1");
        tree.addNode(root, "Child2"); // unused in test, just need two nodes to delete one

        assert(tree.nodeCount() == 3);

        tree.deleteNode(child1);
        assert(tree.nodeCount() == 2);
        assert(root->children->count() == 1);

        tree.clear();
        assert(tree.root == nullptr);
        assert(tree.nodeCount() == 0);

        std::cout << "Deletion test passed!" << std::endl;
    }

    static void testSorting() {
        std::cout << "Testing sorting..." << std::endl;

        EctoSoftTree tree;
        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        tree.addNode(root, "Charlie"); // will be sorted
        tree.addNode(root, "Alpha");   // will be sorted
        tree.addNode(root, "Bravo");   // will be sorted

        // Sort by caption
        auto compare = [](EctoTreeNode* a, EctoTreeNode* b) -> int {
            return a->caption.compare(b->caption);
            };

        root->sort(compare, false);

        assert((*root->children)[0]->caption == "Alpha");
        assert((*root->children)[1]->caption == "Bravo");
        assert((*root->children)[2]->caption == "Charlie");

        std::cout << "Sorting test passed!" << std::endl;
    }

    static void testOnFreeNodeCallback() {
        std::cout << "Testing OnFreeNode callback..." << std::endl;

        int callbackCount = 0;
        EctoSoftTree tree;

        tree.setOnFreeNode([&](EctoTreeNode* node) {
            callbackCount++;
            std::cout << "Callback called for node: " << node->caption << std::endl;
            });

        EctoTreeNode* root = tree.addNode(nullptr, "Root");
        EctoTreeNode* child1 = tree.addNode(root, "Child1");
        EctoTreeNode* child2 = tree.addNode(root, "Child2");

        // Удаление узла должно вызвать callback
        tree.deleteNode(child1);
        assert(callbackCount == 1);

        // Очистка дерева должна вызвать callback для оставшихся узлов
        tree.clear();
        assert(callbackCount == 3); // root + child2

        std::cout << "OnFreeNode callback test passed!" << std::endl;
    }
};
