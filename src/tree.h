/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Tim Severeijns
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <iterator>

/**
 * @brief Класс Tree объявляет базовое дерево, построенное на основе шаблонизированных узлов Node.
 * Каждое дерево состоит из простого корневого узла и ничего больше.
 */
template <typename DataType> class Tree
{
  public:
    class Node;

    class Iterator;
    class PreOrderIterator;
    class PostOrderIterator;
    class LeafIterator;
    class SiblingIterator;

    // Typedefs needed for STL compliance:
    // Определения типов, необходимые для соответствия STL:
    using value_type = Node;
    using reference = Node&;
    using const_reference = const Node&;

    /**
     * @brief Конструктор по умолчанию.
     * 
     */
    Tree() : m_root{ new Node{} }
    {
    }

    /**
     * @brief Конструктор создает новое дерево с предоставленными данными, инкапсулированными в новый узел.
     * 
     */
    Tree(DataType data) : m_root{ new Node{ std::move(data) } }
    {
    }

    /**
     * @brief Copy constructor (Конструктор копирования).
     * 
     */
    Tree(const Tree<DataType>& other) : m_root{ new Node{ *other.m_root } }
    {
    }

    /**
     * @brief Assignment operator (Оператор присваивания).
     * 
     */
    Tree<DataType>& operator=(Tree<DataType> other)
    {
        swap(*this, other);
        return *this;
    }

    /**
     * @brief Swaps all member variables of the left-hand side with that of the right-hand side.
     *  (Меняет местами все переменные-члены левой части на переменные правой части)
     */
    friend void swap(Tree<DataType>& lhs, Tree<DataType>& rhs) noexcept(noexcept(swap(lhs.m_root, rhs.m_root)))
    {
        // Enable Argument Dependent Lookup (ADL):
        // Включить поиск, зависящий от аргумента (ADL):
        using std::swap;

        swap(lhs.m_root, rhs.m_root);
    }

    /**
     * @brief Deletes the root Node, which, in turn, will trigger a deletion of every
     * Node in the Tree. (Удаляет корневой узел, что, в свою очередь, приводит к удалению всех узлов в дереве).
     */
    ~Tree()
    {
        delete m_root;
    }

    /**
     * @returns A pointer to the root Node (Указатель на корневой узел Node)
     * 
     */
    inline Node* GetRoot() const noexcept
    {
        return m_root;
    }

    /**
     * @brief Computes the number of nodes in the Tree.
     * Вычисляет количество узлов в дереве
     *
     * @complexity Linear in the size of the Tree.
     * Линейная сложность в зависимости от размера дерева
     *
     * @returns The total number of nodes in the Tree. This includes leaf and non-leaf nodes,
     * in addition to the root node.
     * 
     * Общее количество узлов в дереве. Сюда входят как листовые, так и нелистовые узлы, а также корневой узел.
     * 
     */
    inline auto Size() const noexcept
    {
        return std::count_if(
            std::begin(*this), std::end(*this), [](const auto&) noexcept { return true; });
    }

    /**
     * @returns The zero-indexed depth of the Node in its Tree.
     * Глубина узла с нулевым индексом в его дереве.
     */
    static unsigned int Depth(const Node& node) noexcept
    {
        unsigned int depth = 0;

        const Node* nodePtr = &node;
        while (nodePtr->GetParent()) {
            ++depth;
            nodePtr = nodePtr->GetParent();
        }

        return depth;
    }

    /**
     * @returns A pre-order iterator that will iterate over all Nodes in the tree.
     * Итератор предварительного заказа, который будет перебирать все узлы в дереве.
     */
    inline typename Tree::PreOrderIterator beginPreOrder() const noexcept
    {
        const auto iterator = Tree<DataType>::PreOrderIterator{ m_root };
        return iterator;
    }

    /**
     * @returns A pre-order iterator pointing "past" the end of the tree.
     * Итератор предварительного заказа, указывающий «за» конец дерева.
     */
    inline typename Tree::PreOrderIterator endPreOrder() const noexcept
    {
        const auto iterator = Tree<DataType>::PreOrderIterator{ nullptr };
        return iterator;
    }

    /**
     * @returns A post-order iterator that will iterator over all nodes in the tree, starting
     * with the root of the Tree.
     * 
     * Итератор пост-заказа, который будет обходить все узлы дерева, начиная с корня дерева.
     */
    inline typename Tree::PostOrderIterator begin() const noexcept
    {
        const auto iterator = Tree<DataType>::PostOrderIterator{ m_root };
        return iterator;
    }

    /**
     * @returns A post-order iterator that points past the end of the Tree.
     * Итератор пост-заказа, указывающий за конец дерева.
     */
    inline typename Tree::PostOrderIterator end() const noexcept
    {
        const auto iterator = Tree<DataType>::PostOrderIterator{ nullptr };
        return iterator;
    }

    /**
     * @returns An iterator that will iterator over all leaf nodes in the Tree, starting with the
     * left-most leaf in the Tree.
     * 
     * Итератор, который будет перебирать все листовые узлы дерева, начиная с самого левого листа дерева.
     * 
     */
    inline typename Tree::LeafIterator beginLeaf() const noexcept
    {
        const auto iterator = Tree<DataType>::LeafIterator{ m_root };
        return iterator;
    }

    /**
     * @return A LeafIterator that points past the last leaf Node in the Tree.
     * Листовой итератор, указывающий за последний листовой узел в дереве.
     */
    inline typename Tree::LeafIterator endLeaf() const noexcept
    {
        const auto iterator = Tree<DataType>::LeafIterator{ nullptr };
        return iterator;
    }

  private:
    Node* m_root{ nullptr };
};

/**
 * Представляет узлы, составляющие дерево.
 *
 * Каждый узел имеет указатель на своего родителя, первого и последнего дочернего узла, 
 *    предыдущего и следующего узла и, конечно же, на данные, которые он инкапсулирует.
 * 
 */
template <typename DataType> class Tree<DataType>::Node
{
  public:
    // Typedefs needed for STL compliance:
    using value_type = DataType;
    using reference = DataType&;
    using const_reference = const DataType&;

    /**
     * @brief Node default constructs a new Node. All outgoing links from this new node will
     * initialized to a nullptr.
     * 
     * Node default constructs a new Node. All outgoing links from this new node will initialized to a nullptr.
     */
    Node() noexcept = default;

    /**
     * @brief Node constructs a new Node encapsulating the specified data. All outgoing links
     * from the node will be initialized to nullptr.
     * 
     * Node создает новый Node, инкапсулирующий указанные данные. Все исходящие ссылки с узла будут инициализированы как nullptr.
     * 
     */
    Node(DataType data) : m_data{ std::move(data) }
    {
    }

    /**
     * @brief Node performs a copy-construction of the specified Node.
     * Node выполняет копирование указанного Node.
     *
     * The nodes in the Node are deep-copied, while the data contained in the tree is
     * shallow-copied.
     * 
     * The nodes in the Node are deep-copied, while the data contained in the tree is shallow-copied.
     * 
     */
    Node(const Node& other) : m_data{ other.m_data }
    {
        Copy(other, *this);
    }

    /**
     * @brief Destroys the Node and all Nodes under it.
     * 
     * Уничтожает узел и все узлы под ним.
     * 
     */
    ~Node()
    {
        DetachFromTree();

        if (m_childCount == 0) {
            m_parent = nullptr;
            m_firstChild = nullptr;
            m_lastChild = nullptr;
            m_previousSibling = nullptr;
            m_nextSibling = nullptr;

            return;
        }

        assert(m_firstChild && m_lastChild);

        Node* childNode = m_firstChild;
        Node* nextNode = nullptr;

        while (childNode != nullptr) {
            nextNode = childNode->m_nextSibling;
            delete childNode;
            childNode = nextNode;
        }

        m_parent = nullptr;
        m_firstChild = nullptr;
        m_lastChild = nullptr;
        m_previousSibling = nullptr;
        m_nextSibling = nullptr;
    }

    /**
     * @brief Assignment operator.
     * 
     * Оператор присваивания
     * 
     */
    Node& operator=(Node other)
    {
        swap(*this, other);
        return *this;
    }

    /**
     * @returns True if the data encapsulated in the left-hand side Node is less than
     * the data encapsulated in the right-hand side Node.
     */
    friend auto operator<(const Node& lhs, const Node& rhs)
    {
        return lhs.GetData() < rhs.GetData();
    }

    /**
     * @returns True if the data encapsulated in the left-hand side Node is less than
     * or equal to the data encapsulated in the right-hand side Node.
     */
    friend auto operator<=(const Node& lhs, const Node& rhs)
    {
        return !(lhs > rhs);
    }

    /**
     * @returns True if the data encapsulated in the left-hand side Node is greater than
     * the data encapsulated in the right-hand side Node.
     */
    friend auto operator>(const Node& lhs, const Node& rhs)
    {
        return rhs < lhs;
    }

    /**
     * @returns True if the data encapsulated in the left-hand side Node is greater than
     * or equal to the data encapsulated in the right-hand side Node.
     */
    friend auto operator>=(const Node& lhs, const Node& rhs)
    {
        return !(lhs < rhs);
    }

    /**
     * @returns True if the data encapsulated in the left-hand side Node is equal to
     * the data encapsulated in the right-hand side Node.
     */
    friend auto operator==(const Node& lhs, const Node& rhs)
    {
        return lhs.GetData() == rhs.GetData();
    }

    /**
     * @returns True if the data encapsulated in the left-hand side Node is not equal
     * to the data encapsulated in the right-hand side Node.
     */
    friend auto operator!=(const Node& lhs, const Node& rhs)
    {
        return !(lhs == rhs);
    }

    /**
     * @brief Swaps all member variables of the left-hand side with that of the right-hand side.
     */
    friend void swap(Node& lhs, Node& rhs) noexcept(noexcept(swap(lhs.m_data, rhs.m_data)))
    {
        // Enable Argument Dependent Lookup (ADL):
        using std::swap;

        swap(lhs.m_parent, rhs.m_parent);
        swap(lhs.m_firstChild, rhs.m_firstChild);
        swap(lhs.m_lastChild, rhs.m_lastChild);
        swap(lhs.m_previousSibling, rhs.m_previousSibling);
        swap(lhs.m_nextSibling, rhs.m_nextSibling);
        swap(lhs.m_data, rhs.m_data);
        swap(lhs.m_childCount, rhs.m_childCount);
        swap(lhs.m_visited, rhs.m_visited);
    }

    /**
     * @brief Detaches and then deletes the Node from the Tree it's part of.
     */
    inline void DeleteFromTree() noexcept
    {
        delete this;
    }

    /**
     * @returns The encapsulated data.
     */
    inline DataType* operator->() noexcept
    {
        return &m_data;
    }

    /**
     * @overload
     */
    inline const DataType* operator->() const noexcept
    {
        return &m_data;
    }

    /**
     * @brief Sets node visitation status.
     *
     * @param[in] visited             Whether the node should be marked as having been visited.
     */
    inline void MarkVisited(const bool visited = true) noexcept
    {
        m_visited = visited;
    }

    /**
     * @returns True if the node has been marked as visited.
     */
    inline bool HasBeenVisited() const noexcept
    {
        return m_visited;
    }

    /**
     * @brief PrependChild will prepend the specified Node as the first child of the Node.
     *
     * @param[in] child               The new Node to set as the first child of the Node.
     *
     * @returns A pointer to the newly appended child.
     */
    inline Node* PrependChild(Node& child) noexcept
    {
        child.m_parent = this;

        if (!m_firstChild) {
            return AddFirstChild(child);
        }

        assert(m_firstChild);

        m_firstChild->m_previousSibling = &child;
        m_firstChild->m_previousSibling->m_nextSibling = m_firstChild;
        m_firstChild = m_firstChild->m_previousSibling;

        m_childCount++;

        return m_firstChild;
    }

    /**
     * @brief Constructs and prepends a new Node as the first child of the
     * Node.
     *
     * @param[in] data                The underlying data to be stored in the new Node.
     *
     * @returns The newly prepended Node.
     */
    inline Node* PrependChild(const DataType& data)
    {
        const auto* newNode = new Node{ data };
        return PrependChild(*newNode);
    }

    /**
     * @overload
     */
    inline Node* PrependChild(DataType&& data)
    {
        auto* const newNode = new Node{ std::move(data) };
        return PrependChild(*newNode);
    }

    /**
     * @brief Appends the specified Node as a child of the Node.
     *
     * @param[in] child               The new Node to set as the last child of the Node.
     *
     * @returns A pointer to the newly appended child.
     */
    inline Node* AppendChild(Node& child) noexcept
    {
        child.m_parent = this;

        if (!m_lastChild) {
            return AddFirstChild(child);
        }

        assert(m_lastChild);

        m_lastChild->m_nextSibling = &child;
        m_lastChild->m_nextSibling->m_previousSibling = m_lastChild;
        m_lastChild = m_lastChild->m_nextSibling;

        m_childCount++;

        return m_lastChild;
    }

    /**
     * @brief Constructs and appends a new Node as the last child of the Node.
     *
     * @param[in] data                The underlying data to be stored in the new Node.
     *
     * @returns The newly appended Node.
     */
    inline Node* AppendChild(const DataType& data)
    {
        auto* const newNode = new Node{ data };
        return AppendChild(*newNode);
    }

    /**
     * @overload
     */
    inline Node* AppendChild(DataType&& data)
    {
        auto* const newNode = new Node{ std::move(data) };
        return AppendChild(*newNode);
    }

    /**
     * @returns The underlying data stored in the Node.
     */
    inline DataType& GetData() noexcept
    {
        return m_data;
    }

    /**
     * @overload
     */
    inline const DataType& GetData() const noexcept
    {
        return m_data;
    }

    /**
     * @returns A pointer to the Node's parent, if it exists; nullptr otherwise.
     */
    inline Node* GetParent() const
    {
        return m_parent;
    }

    /**
     * @returns A pointer to the Node's first child.
     */
    inline Node* GetFirstChild() const
    {
        return m_firstChild;
    }

    /**
     * @returns A pointer to the Node's last child.
     */
    inline Node* GetLastChild() const
    {
        return m_lastChild;
    }

    /**
     * @returns A pointer to the Node's next sibling.
     */
    inline Node* GetNextSibling() const
    {
        return m_nextSibling;
    }

    /**
     * @returns A pointer to the Node's previous sibling.
     */
    inline Node* GetPreviousSibling() const
    {
        return m_previousSibling;
    }

    /**
     * @returns True if this node has direct descendants.
     */
    inline bool HasChildren() const noexcept
    {
        return m_childCount > 0;
    }

    /**
     * @returns The number of direct descendants that this node has.
     *
     * @note This does not include grandchildren.
     */
    inline unsigned int GetChildCount() const noexcept
    {
        return m_childCount;
    }

    /**
     * @returns The total number of descendant nodes belonging to the node.
     */
    inline auto CountAllDescendants() noexcept
    {
        const auto nodeCount = std::count_if(
            Tree<DataType>::PostOrderIterator(this), Tree<DataType>::PostOrderIterator(),
            [](const auto&) noexcept { return true; });

        return nodeCount - 1;
    }

    /**
     * @brief Performs a merge sort of the direct descendants nodes.
     *
     * @param[in] comparator          A callable type to be used as the basis for the sorting
     *                                comparison. This type should be equivalent to:
     *                                   bool comparator(const Node& lhs, const Node& rhs);
     */
    template <typename ComparatorType>
    void SortChildren(const ComparatorType& comparator) noexcept(noexcept(comparator))
    {
        if (!m_firstChild) {
            return;
        }

        Node* head = MergeSort(m_firstChild, comparator);

        if (head->m_parent) {
            head->m_parent->m_firstChild = head;

            Node* temp = head;
            while (temp->m_nextSibling) {
                temp = temp->m_nextSibling;
            }

            head->m_parent->m_lastChild = temp;
        }
    }

  private:
    /**
     * @brief Splits the linked-list of sibling nodes in two.
     * 
     * Разделяет связанный список одноуровневых узлов на две части 
     *
     * @returns The head of the second list.
     * 
     * Возвращается голова (указатель) на второй список
     * 
     */
    Node* Split(Node* head)
    {
        Node* hare = head;
        Node* tortoise = head;

        while (hare->m_nextSibling && hare->m_nextSibling->m_nextSibling) {
            hare = hare->m_nextSibling->m_nextSibling;
            tortoise = tortoise->m_nextSibling;
        }

        Node* secondHead = tortoise->m_nextSibling;
        tortoise->m_nextSibling = nullptr;

        return secondHead;
    }

    /**
     * @brief Main entry point into the merge sort implementation.
     *
     * Основная точка входа в реализацию сортировки слиянием.
     * 
     * @todo A recursive solution may run out of stack space!
     * 
     * Рекурсивному решению может не хватить места в стеке!
     *
     * @param[in] list                The first Node in the list to be sorted. (Первый узел в списке для сортировки.)
     * @param[in] comparator          The comparator function to be called to figure out which node
     *                                is the lesser of the two.
     * 
     * Функция сравнения, вызываемая для определения того, какой узел является меньшим из двух.
     * 
     * 
     */
    template <typename ComparatorType>
    Node* MergeSort(Node*& head, const ComparatorType& comparator) noexcept(noexcept(comparator))
    {
        if (!head || !head->m_nextSibling) {
            return head;
        }

        Node* second = Split(head);

        head = MergeSort(head, comparator);
        second = MergeSort(second, comparator);

        return MergeSortedHalves(head, second, comparator);
    }

    /**
     * @brief Helper function that will merge the sorted halves.
     *
     * Вспомогательная функция, которая объединит отсортированные половинки.
     * 
     * @param[in] lhs                 The first node of the sorted left half.  (Первый узел отсортированной левой половины.)
     * @param[in] rhs                 The first node of the sorted right half. (Первый узел отсортированной правой половины.)
     *
     * @returns The first node of the merged Node list. (Первый узел объединенного списка узлов.)
     */
    template <typename ComparatorType>
    Node* MergeSortedHalves(Node* lhs, Node* rhs, const ComparatorType& comparator) noexcept(
        noexcept(comparator))
    {
        Node* head = nullptr;
        if (comparator(*lhs, *rhs)) {
            head = lhs;
            lhs = lhs->m_nextSibling;
        } else {
            head = rhs;
            rhs = rhs->m_nextSibling;
        }

        head->m_previousSibling = nullptr;

        Node* tail = head;
        Node* previous = nullptr;

        while (lhs && rhs) {
            previous = tail;

            if (comparator(*lhs, *rhs)) {
                tail->m_nextSibling = lhs;
                lhs = lhs->m_nextSibling;
            } else {
                tail->m_nextSibling = rhs;
                rhs = rhs->m_nextSibling;
            }

            tail = tail->m_nextSibling;
            tail->m_previousSibling = previous;
        }

        previous = tail;
        tail->m_nextSibling = lhs ? lhs : rhs;
        tail = tail->m_nextSibling;
        tail->m_previousSibling = previous;

        return head;
    }

    /**
     * @brief Helper function to make it easier to add the first descendant.
     * 
     * Вспомогательная функция, упрощающая добавление первого потомка.
     *
     * @param[in] child               The Node to be added as a child. ( Узел, который будет добавлен как дочерний. )
     *
     * @returns The newly added node. (Недавно добавленный узел.)
     */
    inline Node* AddFirstChild(Node& child) noexcept
    {
        assert(m_childCount == 0);

        m_firstChild = &child;
        m_lastChild = m_firstChild;

        m_childCount++;

        return m_firstChild;
    }

    /**
     * @brief Helper function to recursively copy the specified |source| Node and all its
     * descendants.
     *
     * Вспомогательная функция для рекурсивного копирования указанного |источника| Node и все его потомки.
     * 
     * @param[in] source              The Node to copy information from. (Узел, с которого копируется информация.)
     * @param[out] sink               The Node to place a copy of the information into. (Узел, в который нужно поместить копию информации.)
     */
    void Copy(const Node& source, Node& sink)
    {
        if (!source.HasChildren()) {
            return;
        }

        std::for_each(
            Tree<DataType>::SiblingIterator(source.GetFirstChild()),
            Tree<DataType>::SiblingIterator(),
            [&](Tree<DataType>::const_reference node) { sink.AppendChild(node.GetData()); });

        auto sourceItr = Tree<DataType>::SiblingIterator{ source.GetFirstChild() };
        auto sinkItr = Tree<DataType>::SiblingIterator{ sink.GetFirstChild() };

        const auto end = Tree<DataType>::SiblingIterator{};
        while (sourceItr != end) {
            Copy(*sourceItr++, *sinkItr++);
        }
    }

    /**
     * @brief Removes the Node from the tree structure, updating all surrounding links
     * as appropriate.
     *
     * Удаляет узел из древовидной структуры, обновляя все окружающие ссылки соответствующим образом.
     * 
     * @note This function does not actually delete the node. (Эта функция фактически не удаляет узел.)
     *
     * @returns A pointer to the detached Node. This returned Node can safely be deleted
     * once detached.
     * 
     * Указатель на отсоединенный Node. Этот возвращенный узел можно безопасно удалить после отсоединения.
     * 
     */
    Node* DetachFromTree() noexcept
    {
        if (m_previousSibling && m_nextSibling) {
            m_previousSibling->m_nextSibling = m_nextSibling;
            m_nextSibling->m_previousSibling = m_previousSibling;
        } else if (m_previousSibling) {
            m_previousSibling->m_nextSibling = nullptr;
        } else if (m_nextSibling) {
            m_nextSibling->m_previousSibling = nullptr;
        }

        if (!m_parent) {
            return this;
        }

        if (m_parent->m_firstChild == m_parent->m_lastChild) {
            m_parent->m_firstChild = nullptr;
            m_parent->m_lastChild = nullptr;
        } else if (m_parent->m_firstChild == this) {
            assert(m_parent->m_firstChild->m_nextSibling);
            m_parent->m_firstChild = m_parent->m_firstChild->m_nextSibling;
        } else if (m_parent->m_lastChild == this) {
            assert(m_parent->m_lastChild->m_previousSibling);
            m_parent->m_lastChild = m_parent->m_lastChild->m_previousSibling;
        }

        m_parent->m_childCount--;

        return this;
    }

    Node* m_parent{ nullptr };
    Node* m_firstChild{ nullptr };
    Node* m_lastChild{ nullptr };
    Node* m_previousSibling{ nullptr };
    Node* m_nextSibling{ nullptr };

    DataType m_data{};

    unsigned int m_childCount{ 0 };

    bool m_visited{ false };
};

/**
 * @brief The base iterator.
 * 
 * Базовый итератор
 *
 * This is the base iterator class that all other iterators (sibling, leaf, post-, pre-, and
 * in-order) will derive from. This class can only instantiated by derived types.
 * 
 * Это базовый класс итератора, от которого будут наследоваться все остальные итераторы 
 * (родственные, листовые, пост-, предварительные и порядковые). 
 *  Этот класс может создаваться только производными типами.
 * 
 * 
 * 
 */
template <typename DataType> class Tree<DataType>::Iterator
{
  public:
    // Typedefs needed for STL compliance:
    using value_type = DataType;
    using pointer = DataType*;
    using reference = DataType&;
    using const_reference = const DataType&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator_category = std::forward_iterator_tag;

    /**
     * @returns True if the Tree::Iterator points to a valid Node; false otherwise.
     */
    explicit operator bool() const noexcept
    {
        return m_currentNode != nullptr;
    }

    /**
     * @returns The Node pointed to by the Tree::Iterator.
     */
    inline Node& operator*() noexcept
    {
        return *m_currentNode;
    }

    /**
     * @overload
     */
    inline const Node& operator*() const noexcept
    {
        return *m_currentNode;
    }

    /**
     * @returns A pointer to the Node.
     */
    inline Node* operator&() noexcept
    {
        return m_currentNode;
    }

    /**
     * @overload
     */
    inline const Node* operator&() const noexcept
    {
        return m_currentNode;
    }

    /**
     * @returns A pointer to the Node pointed to by the Tree::Iterator.
     */
    inline Node* operator->() noexcept
    {
        return m_currentNode;
    }

    /**
     * @overload
     */
    inline const Node* operator->() const noexcept
    {
        return m_currentNode;
    }

    /**
     * @returns True if the iterator points to the same node as the other iterator,
     * and false otherwise.
     */
    inline bool operator==(const Iterator& other) const
    {
        return m_currentNode == other.m_currentNode;
    }

    /**
     * @returns True if the iterator points to the same node as the other iterator,
     * and false otherwise.
     */
    bool operator!=(const Iterator& other) const noexcept
    {
        return m_currentNode != other.m_currentNode;
    }

  protected:
    /**
     * Default constructor.
     */
    Iterator() noexcept = default;

    /**
     * Copy constructor.
     */
    explicit Iterator(const Iterator& other) = default;

    /**
     * Constructs a iterator starting at the specified node.
     */
    explicit Iterator(const Node* node) noexcept
        : m_currentNode{ const_cast<Node*>(node) }, m_startingNode{ const_cast<Node*>(node) }
    {
    }

    Node* m_currentNode{ nullptr };

    const Node* m_startingNode{ nullptr };
    const Node* m_endingNode{ nullptr };
};

/**
 * @brief A pre-order tree iterator.
 */
template <typename DataType>
class Tree<DataType>::PreOrderIterator final : public Tree<DataType>::Iterator
{
  public:
    /**
     * Default constructor.
     */
    PreOrderIterator() noexcept = default;

    /**
     * Constructs an iterator that starts and ends at the specified node.
     */
    explicit PreOrderIterator(const Node* node) noexcept : Iterator{ node }
    {
        if (!node) {
            return;
        }

        if (node->GetNextSibling()) {
            this->m_endingNode = node->GetNextSibling();
        } else {
            this->m_endingNode = node;
            while (this->m_endingNode->GetParent() &&
                   !this->m_endingNode->GetParent()->GetNextSibling()) {
                this->m_endingNode = this->m_endingNode->GetParent();
            }

            if (this->m_endingNode->GetParent()) {
                this->m_endingNode = this->m_endingNode->GetParent()->GetNextSibling();
            } else {
                this->m_endingNode = nullptr;
            }
        }
    }

    /**
     * Pre-fix increment operator.
     */
    typename Tree::PreOrderIterator& operator++() noexcept
    {
        assert(this->m_currentNode);
        auto* traversingNode = this->m_currentNode;

        if (traversingNode->HasChildren()) {
            traversingNode = traversingNode->GetFirstChild();
        } else if (traversingNode->GetNextSibling()) {
            traversingNode = traversingNode->GetNextSibling();
        } else {
            while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling()) {
                traversingNode = traversingNode->GetParent();
            }

            if (traversingNode->GetParent()) {
                traversingNode = traversingNode->GetParent()->GetNextSibling();
            } else {
                traversingNode = nullptr;
            }
        }

        this->m_currentNode = (traversingNode != this->m_endingNode) ? traversingNode : nullptr;
        return *this;
    }

    /**
     * Post-fix increment operator.
     */
    typename Tree::PreOrderIterator operator++(int) noexcept
    {
        const auto result = *this;
        ++(*this);

        return result;
    }
};

/**
 * @brief A post-order tree iterator.
 */
template <typename DataType>
class Tree<DataType>::PostOrderIterator final : public Tree<DataType>::Iterator
{
  public:
    /**
     * Default constructor.
     */
    PostOrderIterator() noexcept = default;

    /**
     * Constructs an iterator that starts and ends at the specified node.
     */
    explicit PostOrderIterator(const Node* node) noexcept : Iterator{ node }
    {
        if (!node) {
            return;
        }

        // Compute and set the starting node:

        auto* traversingNode = node;
        while (traversingNode->GetFirstChild()) {
            traversingNode = traversingNode->GetFirstChild();
        }

        assert(traversingNode);
        this->m_currentNode = const_cast<Node*>(traversingNode);

        // Commpute and set the ending node:

        if (node->GetNextSibling()) {
            traversingNode = node->GetNextSibling();
            while (traversingNode->HasChildren()) {
                traversingNode = traversingNode->GetFirstChild();
            }

            this->m_endingNode = traversingNode;
        } else {
            this->m_endingNode = node->GetParent();
        }
    }

    /**
     * Pre-fix increment operator.
     */
    typename Tree::PostOrderIterator& operator++() noexcept
    {
        assert(this->m_currentNode);
        auto* traversingNode = this->m_currentNode;

        if (traversingNode->HasChildren() && !m_traversingUpTheTree) {
            while (traversingNode->GetFirstChild()) {
                traversingNode = traversingNode->GetFirstChild();
            }
        } else if (traversingNode->GetNextSibling()) {
            m_traversingUpTheTree = false;

            traversingNode = traversingNode->GetNextSibling();
            while (traversingNode->HasChildren()) {
                traversingNode = traversingNode->GetFirstChild();
            }
        } else {
            m_traversingUpTheTree = true;

            traversingNode = traversingNode->GetParent();
        }

        this->m_currentNode = (traversingNode != this->m_endingNode) ? traversingNode : nullptr;
        return *this;
    }

    /**
     * Post-fix increment operator.
     */
    typename Tree::PostOrderIterator operator++(int) noexcept
    {
        const auto result = *this;
        ++(*this);

        return result;
    }

  private:
    bool m_traversingUpTheTree{ false };
};

/**
 * @brief A leaf-order tree iterator.
 */
template <typename DataType>
class Tree<DataType>::LeafIterator final : public Tree<DataType>::Iterator
{
  public:
    /**
     * Default constructor.
     */
    LeafIterator() noexcept = default;

    /**
     * Constructs an iterator that starts at the specified node and iterates to the end.
     */
    explicit LeafIterator(const Node* node) noexcept : Iterator{ node }
    {
        if (!node) {
            return;
        }

        // Compute and set the starting node:

        if (node->HasChildren()) {
            auto* firstNode = node;
            while (firstNode->GetFirstChild()) {
                firstNode = firstNode->GetFirstChild();
            }

            this->m_currentNode = const_cast<Node*>(firstNode);
        }

        // Compute and set the ending node:

        if (node->GetNextSibling()) {
            auto* lastNode = node->GetNextSibling();
            while (lastNode->HasChildren()) {
                lastNode = lastNode->GetFirstChild();
            }

            this->m_endingNode = lastNode;
        } else {
            this->m_endingNode = node;
            while (this->m_endingNode->GetParent() &&
                   !this->m_endingNode->GetParent()->GetNextSibling()) {
                this->m_endingNode = this->m_endingNode->GetParent();
            }

            if (this->m_endingNode->GetParent()) {
                this->m_endingNode = this->m_endingNode->GetParent()->GetNextSibling();
                while (this->m_endingNode->HasChildren()) {
                    this->m_endingNode = this->m_endingNode->GetFirstChild();
                }
            } else {
                this->m_endingNode = nullptr;
            }
        }
    }

    /**
     * Pre-fix increment operator.
     */
    typename Tree::LeafIterator& operator++() noexcept
    {
        assert(this->m_currentNode);
        auto* traversingNode = this->m_currentNode;

        if (traversingNode->HasChildren()) {
            while (traversingNode->GetFirstChild()) {
                traversingNode = traversingNode->GetFirstChild();
            }
        } else if (traversingNode->GetNextSibling()) {
            traversingNode = traversingNode->GetNextSibling();

            while (traversingNode->GetFirstChild()) {
                traversingNode = traversingNode->GetFirstChild();
            }
        } else if (traversingNode->GetParent()) {
            while (traversingNode->GetParent() && !traversingNode->GetParent()->GetNextSibling()) {
                traversingNode = traversingNode->GetParent();
            }

            if (traversingNode->GetParent()) {
                traversingNode = traversingNode->GetParent()->GetNextSibling();

                while (traversingNode && traversingNode->HasChildren()) {
                    traversingNode = traversingNode->GetFirstChild();
                }
            } else {
                traversingNode = nullptr;
            }
        }

        this->m_currentNode = (traversingNode != this->m_endingNode) ? traversingNode : nullptr;
        return *this;
    }

    /**
     * Post-fix increment operator.
     */
    typename Tree::LeafIterator operator++(int) noexcept
    {
        const auto result = *this;
        ++(*this);

        return result;
    }
};

/**
 * @brief A sibling node iterator.
 * 
 * Итератор родственного узла
 * 
 */
template <typename DataType>
class Tree<DataType>::SiblingIterator final : public Tree<DataType>::Iterator
{
  public:
    /**
     * Default constructor.
     * 
     * Конструктор по умолчанию
     * 
     */
    SiblingIterator() noexcept = default;

    /**
     * Constructs an iterator that starts at the specified node and iterates to the end.
     * 
     * Создает итератор, который начинается с указанного узла и выполняет итерацию до конца.
     * 
     * 
     */
    explicit SiblingIterator(const Node* node) noexcept : Iterator{ node }
    {
    }

    /**
     * Pre-fix increment operator.
     * 
     * Оператор увеличения префикса.
     * 
     */
    typename Tree::SiblingIterator& operator++() noexcept
    {
        if (this->m_currentNode) {
            this->m_currentNode = this->m_currentNode->GetNextSibling();
        }

        return *this;
    }

    /**
     * Post-fix increment operator.
     * 
     * Оператор приращения после исправления
     * 
     */
    typename Tree::SiblingIterator operator++(int) noexcept
    {
        const auto result = *this;
        ++(*this);

        return result;
    }
};
