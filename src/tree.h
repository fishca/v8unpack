#ifndef TREE_H
#define TREE_H


#include <iostream>
#include <stack>
#include <vector>
#include <string>
#include <sstream>

// Узел дерева
struct sTreeNode {
    enum class Type {
        OBJECT,    // Объект { ... }
        STRING,    // Строка (GUID, текст, кириллица)
        NUMBER     // Число
    };

    Type type;
    std::vector<std::unique_ptr<sTreeNode>> children;
    //std::string value;
    std::wstring value;

    sTreeNode(Type t = Type::OBJECT, const std::wstring& val = L"") : type(t), value(val) 
    {}

    // Добавление дочернего узла
    sTreeNode* addChild(std::unique_ptr<sTreeNode> child) {
        children.push_back(std::move(child));
        return children.back().get();
    }

    // Добавление строкового узла
    sTreeNode* addString(const std::wstring& str) {
        auto node = std::make_unique<sTreeNode>(Type::STRING, str);
        return addChild(std::move(node));
    }

    // Добавление числового узла
    sTreeNode* addNumber(const std::wstring& num) {
        auto node = std::make_unique<sTreeNode>(Type::NUMBER, num);
        return addChild(std::move(node));
    }

    // Рекурсивный вывод дерева
    void print(int depth = 0) const {
        std::wstring indent(depth * 2, ' ');

        switch (type) {
        case Type::OBJECT:
            std::wcout << indent << "Object (" << children.size() << " children):" << std::endl;
            for (const auto& child : children) {
                child->print(depth + 1);
            }
            break;
        case Type::STRING:
            std::wcout << indent << "String: \"" << value << "\"" << std::endl;
            break;
        case Type::NUMBER:
            std::wcout << indent << "Number: " << value << std::endl;
            break;
        }
    }

    // Получение типа как строки
    std::string getTypeString() const {
        switch (type) {
        case Type::OBJECT: return "object";
        case Type::STRING: return "string";
        case Type::NUMBER: return "number";
        default: return "unknown";
        }
    }

    // Получение дерева в виде строки (JSON-like формат)
    std::wstring toString(int depth = 0) const {
        std::wstringstream ss;
        std::wstring indent(depth * 2, ' ');

        switch (type) {
        case Type::OBJECT:
            ss << indent << "{\n";
            for (size_t i = 0; i < children.size(); ++i) {
                ss << children[i]->toString(depth + 1);
                if (i < children.size() - 1) {
                    ss << ",\n";
                }
            }
            ss << "\n" << indent << "}";
            break;
        case Type::STRING:
            ss << indent << "\"" << value << "\"";
            break;
        case Type::NUMBER:
            ss << indent << value;
            break;
        }
        return ss.str();
    }
};

class BracketParser {
private:
    
    std::stack<char> bracketStack;
    std::stack<sTreeNode*> nodeStack;
    std::unique_ptr<sTreeNode> root;
    std::wstring currentToken;
    
    bool inString;
    bool isGuidMode;
    bool inQuotedString;
    char quoteChar;

    // Проверка валидности символа (расширена для Unicode)
    bool isValidChar(unsigned char c) const {
        // Разрешаем все печатные символы ASCII и UTF-8 байты
        return (c >= 32 && c <= 126) || c == '\n' || c == '\r' || c == '\t' || (c >= 0xC0 && c <= 0xFF);
    }

    // Проверка, является ли байт началом UTF-8 последовательности
    bool isUtf8StartByte(unsigned char c) const {
        return (c >= 0xC0 && c <= 0xDF) || (c >= 0xE0 && c <= 0xEF) || (c >= 0xF0 && c <= 0xF7);
    }

    // Проверка, является ли байт продолжением UTF-8 последовательности
    bool isUtf8ContinuationByte(unsigned char c) const {
        return (c >= 0x80 && c <= 0xBF);
    }

    // Проверка GUID формата
    bool isGuid(const std::string& str) const {
        // Формат: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (8-4-4-4-12)
        if (str.length() != 36) return false;

        for (size_t i = 0; i < str.length(); ++i) {
            char c = str[i];
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                if (c != '-') return false;
            }
            else {
                if (!std::isxdigit(c)) return false;
            }
        }
        return true;
    }

    // Проверка GUID формата
    bool isGuid(const std::wstring& str) const {
        // Формат: xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx (8-4-4-4-12)
        if (str.length() != 36) return false;

        for (size_t i = 0; i < str.length(); ++i) {
            char c = str[i];
            if (i == 8 || i == 13 || i == 18 || i == 23) {
                if (c != '-') return false;
            }
            else {
                if (!std::isxdigit(c)) return false;
            }
        }
        return true;
    }


    // Проверка числа
    bool isNumber(const std::string& str) const {
        if (str.empty()) 
            return false;
        
        for (char c : str) {
            if (!std::isdigit(static_cast<unsigned char>(c))) 
                return false;
        }
        return true;
    }

    bool isNumber(const std::wstring& str) const {
        if (str.empty()) 
            return false;
        
        for (char c : str) {
            if (!std::isdigit(static_cast<unsigned char>(c))) 
                return false;
        }
        return true;
    }


    // Обработка завершения токена
    void processToken() {
        if (!currentToken.empty()) {
            if (!nodeStack.empty()) {
                // Определяем тип токена
                if (isNumber(currentToken)) {
                    nodeStack.top()->addNumber(currentToken);
                }
                else {
                    nodeStack.top()->addString(currentToken);
                }
            }
            currentToken.clear();
        }
        inString = false;
        isGuidMode = false;
        inQuotedString = false;
    }

    // Обработка строки в кавычках
    void processQuotedString(char c) {
        if (!inQuotedString) {
            // Начало строки в кавычках
            inQuotedString = true;
            quoteChar = c;
            currentToken.clear();
        }
        else if (c == quoteChar) {
            // Конец строки в кавычках
            inQuotedString = false;
            if (!nodeStack.empty()) {
                nodeStack.top()->addString(currentToken);
            }
            currentToken.clear();
        }
        else {
            // Символ внутри строки в кавычках
            currentToken += c;
        }
    }

    void processQuotedStringW(wchar_t c) {
        if (!inQuotedString) {
            // Начало строки в кавычках
            inQuotedString = true;
            quoteChar = c;
            currentToken.clear();
        }
        else if (c == quoteChar) {
            // Конец строки в кавычках
            inQuotedString = false;
            if (!nodeStack.empty()) {
                nodeStack.top()->addString(currentToken);
            }
            currentToken.clear();
        }
        else {
            // Символ внутри строки в кавычках
            currentToken += c;
        }
    }


public:
    BracketParser() : inString(false), isGuidMode(false), inQuotedString(false), quoteChar('"') {}

    // Парсинг строки и построение дерева
    bool parse(const std::string& input) {
        // Очистка состояния
        while (!bracketStack.empty()) 
            bracketStack.pop();
        while (!nodeStack.empty()) 
            nodeStack.pop();
        
        root.reset();
        currentToken.clear();
        inString = false;
        isGuidMode = false;
        inQuotedString = false;
        quoteChar = '"';

        // Создание корневого узла
        root = std::make_unique<sTreeNode>();
        nodeStack.push(root.get());

        for (size_t i = 0; i < input.length(); ++i) {
            unsigned char c = static_cast<unsigned char>(input[i]);

            // Проверка валидности символа
            if (!isValidChar(c)) {
                // Для отладки выведем проблемный символ
                std::cout << "Warning: Skipping invalid character at position " << i
                    << " (code: " << static_cast<int>(c) << ")" << std::endl;
                continue;
            }

            if (inQuotedString) {
                // Обработка строки в кавычках
                processQuotedString(c);
            }
            else if (c == '"' || c == '\'') {
                // Начало или конец строки в кавычках
                processQuotedString(c);
            }
            else if (c == '{') {
                processToken(); // Сохраняем текущий токен перед открытием новой скобки

                // Создаем новый узел для вложенной структуры
                auto newNode = std::make_unique<sTreeNode>(sTreeNode::Type::OBJECT);
                sTreeNode* newNodePtr = nodeStack.top()->addChild(std::move(newNode));

                // Добавляем в стек для последующего заполнения
                nodeStack.push(newNodePtr);
                bracketStack.push(c);

            }
            else if (c == '}') {
                processToken(); // Сохраняем текущий токен перед закрытием скобки

                if (bracketStack.empty()) {
                    return false; // Закрывающая скобка без открывающей
                }

                bracketStack.pop();
                nodeStack.pop(); // Завершаем текущий узел

            }
            else if (c == ',') {
                // Запятая разделяет элементы - сохраняем текущий токен
                processToken();

            }
            else {
                // Обработка содержимого токена
                if (!inString && !std::isspace(c)) {
                    inString = true;
                    // Проверяем, не начинается ли GUID
                    if (i + 36 <= input.length()) {
                        std::string potentialGuid = input.substr(i, 36);
                        if (isGuid(potentialGuid)) {
                            isGuidMode = true;
                        }
                    }
                }

                if (!std::isspace(c) || inString) {
                    currentToken += c;
                }

                // Если мы в режиме GUID, обрабатываем его целиком
                if (isGuidMode && currentToken.length() == 36) {
                    processToken();
                }
            }
        }

        processToken(); // Обрабатываем последний токен

        return bracketStack.empty() && nodeStack.size() == 1;
    }

    // Парсинг строки и построение дерева
    bool parse(const std::wstring& input) {
        // Очистка состояния
        while (!bracketStack.empty())
            bracketStack.pop();
        while (!nodeStack.empty())
            nodeStack.pop();

        root.reset();
        currentToken.clear();
        inString = false;
        isGuidMode = false;
        inQuotedString = false;
        quoteChar = '"';

        // Создание корневого узла
        root = std::make_unique<sTreeNode>();
        nodeStack.push(root.get());

        for (size_t i = 0; i < input.length(); ++i) {
            wchar_t c = static_cast<wchar_t>(input[i]);

            // Проверка валидности символа
            //if (!isValidChar(c)) {
            //    // Для отладки выведем проблемный символ
            //    std::cout << "Warning: Skipping invalid character at position " << i
            //        << " (code: " << static_cast<int>(c) << ")" << std::endl;
            //    continue;
            //}

            if (inQuotedString) {
                // Обработка строки в кавычках
                processQuotedStringW(c);
            }
            else if (c == '"' || c == '\'') {
                // Начало или конец строки в кавычках
                processQuotedStringW(c);
            }
            else if (c == '{') {
                processToken(); // Сохраняем текущий токен перед открытием новой скобки

                // Создаем новый узел для вложенной структуры
                auto newNode = std::make_unique<sTreeNode>(sTreeNode::Type::OBJECT);
                sTreeNode* newNodePtr = nodeStack.top()->addChild(std::move(newNode));

                // Добавляем в стек для последующего заполнения
                nodeStack.push(newNodePtr);
                bracketStack.push(c);

            }
            else if (c == '}') {
                processToken(); // Сохраняем текущий токен перед закрытием скобки

                if (bracketStack.empty()) {
                    return false; // Закрывающая скобка без открывающей
                }

                bracketStack.pop();
                nodeStack.pop(); // Завершаем текущий узел

            }
            else if (c == ',') {
                // Запятая разделяет элементы - сохраняем текущий токен
                processToken();

            }
            else {
                // Обработка содержимого токена
                if (!inString && !std::isspace(c)) {
                    inString = true;
                    // Проверяем, не начинается ли GUID
                    if (i + 36 <= input.length()) {
                        std::wstring potentialGuid = input.substr(i, 36);
                        if (isGuid(potentialGuid)) {
                            isGuidMode = true;
                        }
                    }
                }

                if (!std::isspace(c) || inString) {
                    currentToken += c;
                }

                // Если мы в режиме GUID, обрабатываем его целиком
                if (isGuidMode && currentToken.length() == 36) {
                    processToken();
                }
            }
        }

        processToken(); // Обрабатываем последний токен

        return bracketStack.empty() && nodeStack.size() == 1;
    }


    // Получение корня дерева
    sTreeNode* getRoot() const {
        return root.get();
    }

    // Валидация без построения дерева
    bool isValid(const std::string& input) {
        try {
            return parse(input);
        }
        catch (const std::exception&) {
            return false;
        }
    }

    bool isValid(const std::wstring& input) {
        try {
            return parse(input);
        }
        catch (const std::exception&) {
            return false;
        }
    }


    // Вывод дерева
    void printTree() const {
        if (root) {
            root->print();
        }
        else {
            std::cout << "Tree is empty" << std::endl;
        }
    }

    // Получение дерева в виде строки
    std::wstring treeToString() const {
        if (root && !root->children.empty()) {
            return root->children[0]->toString(); // Пропускаем корневой контейнер
        }
        return L"{}";
    }

    // Поиск всех GUID'ов в дереве
    //std::vector<std::string> findGuids() const {
    //    std::vector<std::string> guids;
    //    if (root) {
    //        findGuidsRecursive(root.get(), guids);
    //    }
    //    return guids;
    //}

    // Поиск всех строк с кириллицей
    //std::vector<std::string> findCyrillicStrings() const {
    //    std::vector<std::string> cyrillicStrings;
    //    if (root) {
    //        findCyrillicRecursive(root.get(), cyrillicStrings);
    //    }
    //    return cyrillicStrings;
    //}

private:
    //void findGuidsRecursive(sTreeNode* node, std::vector<std::string>& guids) const {
    //    if (node->type == sTreeNode::Type::STRING && isGuid(node->value)) {
    //        guids.push_back(node->value);
    //    }

    //    for (const auto& child : node->children) {
    //        findGuidsRecursive(child.get(), guids);
    //    }
    //}

    // Проверка содержит ли строка кириллицу
    //bool containsCyrillic(const std::string& str) const {
    //    for (unsigned char c : str) {
    //        // Кириллица в UTF-8: диапазоны 0xD090-0xD0BF и 0xD180-0xD1BF
    //        if (c >= 0xD0 && c <= 0xD1) {
    //            return true;
    //        }
    //    }
    //    return false;
    //}

    //void findCyrillicRecursive(sTreeNode* node, std::vector<std::string>& cyrillicStrings) const {
    //    if (node->type == sTreeNode::Type::STRING && containsCyrillic(node->value)) {
    //        cyrillicStrings.push_back(node->value);
    //    }

    //    for (const auto& child : node->children) {
    //        findCyrillicRecursive(child.get(), cyrillicStrings);
    //    }
    //}
};



// Класс TreeNode представляет узел дерева
class TreeNode {
public:
    std::wstring value; // Данные узла
    std::vector<TreeNode*> children; // Вектор для хранения подчиненных узлов

    // Конструктор
    TreeNode(const std::wstring& val) : value(val) {}

    // Деструктор, освобождающий память, выделенную подчиненным узлам
    ~TreeNode();

    // Метод для добавления подчиненного узла
    void addChild(TreeNode* child);

    // Метод для вывода дерева
    void print(int level = 0) const;
};

#endif // TREE_H