# Справочник Классов V8Unpack

## Краткий Обзор

Этот документ содержит подробное описание всех основных классов проекта V8Unpack, их методы, назначение и взаимосвязи.

## Application Layer (`src/app/`)

### ArgumentParser

**Назначение:**
Класс для парсинга аргументов командной строки с поддержкой POSIX-style опций и legacy команд.

**Основные методы:**
- `parse(int argc, char* argv[])`: Парсит аргументы ОС
- `parse(std::vector<std::string>& args)`: Парсит вектор строк
- `extractCommand(const std::string&)`: Извлекает имя команды
- `parseOption(const std::string&)`: Парсит опции --key=value

**Структуры данных:**

#### ParsedArgs
```cpp
struct ParsedArgs {
    std::string command;                    // Имя команды
    std::vector<std::string> args;         // Позиционные аргументы
    std::map<std::string, std::string> options; // Опции --key=value
    bool hasListFile;                     // Есть ли файл списка команд
    std::string listFilePath;             // Путь к файлу списка

    // Методы доступа
    std::string getOption(const std::string& key, const std::string& defaultValue = "") const;
    bool hasOption(const std::string& key) const;
    size_t argCount() const;
    bool empty() const;
};
```

**Примеры использования:**
```bash
# Legacy команда
v8unpack -BUILD input_dir output.cf
# Преобразуется в
ParsedArgs {
  command: "build",
  args: ["input_dir", "output.cf"]
}
```

### Application

**Назначение:**
Главная фасадная компонента приложения, координирующая все подсистемы по паттерну Facade.

**Жизненный цикл:**
1. `initialize()`: Создание зависимостей через DI
2. `run()`: Основной цикл выполнения
3. `executeCoreLogic()`: Обработка команд
4. `handleError()`: Централизованная обработка ошибок

**Ключевые методы:**
- `run(int argc, char* argv[])`: Точка входа
- `executeCommand(ParsedArgs&)`: Выполнение команд
- `executeLegacyCommand(ParsedArgs&)`: Обратная совместимость
- `handleSpecialCases(ParsedArgs&)`: Help, version и т.д.

**Структура ExecutionMetrics:**
```cpp
struct ExecutionMetrics {
    std::string startTime;
    int commandsExecuted = 0;
    int errorsEncountered = 0;
};
```

## Command System (`src/commands/`)

### Command (Abstract Base Class)

**Интерфейс:**
```cpp
class Command {
protected:
    std::shared_ptr<MessageRegistrator> logger_;

public:
    virtual ~Command() = default;

    virtual int execute(const std::vector<std::string>& args) = 0;
    virtual std::string getName() const = 0;
    virtual std::string getDescription() const = 0;
    virtual void showUsage() const = 0;

    virtual bool supportsModifier(const std::string& modifier) const;
    virtual void setModifier(const std::string& modifier, bool value);
};
```

**Паттерн:** Strategy Pattern - каждая команда инкапсулирует алгоритм выполнения.

### CommandRegistry

**Назначение:**
Централизованный реестр команд по паттерну Registry. Управляет жизненным циклом команд.

**Основные методы:**
- `registerCommand(std::unique_ptr<Command>)`: Регистрация команды
- `getCommand(const std::string&)`: Получение команды по имени
- `hasCommand(const std::string&)`: Проверка существования
- `showHelp()`: Отображение справки по всем командам

**Пример использования:**
```cpp
auto registry = CommandFactory(logger_).createRegistry();
// Регистрирует все команды автоматически
registry->showHelp(); // Показывает все доступные команды
```

### CommandFactory

**Назначение:**
Фабрика команд по паттерну Factory. Отвечает за создание конкретных реализаций команд с правильными зависимостями.

**Методы создания:**
- `createUnpackCommand()`: Создает UnpackCommand
- `createBuildCommand()`: Создает BuildCommand
- `createListCommand()`: Создает ListCommand
- `createRegistry()`: Создает полный реестр команд

**Dependency Injection:**
Все команды получают MessageRegistrator через конструктор для логирования.

### Конкретные Реализации Команд

#### BuildCommand

**Наследование:** Command → BuildCommand

**Назначение:**
Сборка конфигурационных файлов (.cf) из директории файлов.

**Методы:**
- `execute(std::vector<std::string>&)`: Выполнение сборки
- `supportsModifier("-nopack")`: Поддержка модификатора
- `setModifier()`: Установка флага no-pack

**Взаимосвязи:**
- Вызывает `V8File::BuildCfFile(dirname, filename, !dont_deflate)`
- Проверяет совместимость через `directory_container_compatibility()`

#### ListCommand (ListFiles)

**Назначение:**
Отображение содержимого V8 файла без распаковки.

**Взаимосвязи:**
- Использует `V8File::ListFiles(filename)`
- Работа с Format15/Format16 автоматически

## Core Infrastructure (`src/`)

### V8File

**Назначение:**
Центральный класс для работы с V8 файлами конфигурации 1C:Enterprise.

**Основные структуры:**

#### CV8Elem
```cpp
class CV8Elem {
    std::vector<char> header;
    std::vector<char> data;
    bool IsV8File = false;
    mutable CV8File UnpackedData;

    std::string GetName() const;
    void SetName(const std::string&);

    bool Pack(bool deflate = true);  // Сжатие данных
    void Dispose();                  // Очистка ресурсов
};
```

#### CV8File
```cpp
class CV8File {
    stFileHeader FileHeader;
    std::vector<stElemAddr> ElemsAddrs;
    std::vector<CV8Elem> Elems;
    bool IsDataPacked = true;

    int LoadFileFromFolder(const std::string& dirname);
    int GetData(std::vector<char>& data);
};
```

**Ключевые глобальные функции:**
- `int BuildCfFile(const std::string& in_dir, const std::string& out, bool dont_deflate)`
- `int ListFiles(const std::string& filename)`
- `int Parse(const std::string& in, const std::string& out, std::vector<std::string>& filter)`
- `int UnpackToFolder(const std::string& in, const std::string& out, std::string& elem, bool print)`

**Форматы файлов:**
- Format15: Legacy формат
- Format16: 8.3.16+ формат с расширенными возможностями

### VersionFile

**Назначение:**
Управление версиями V8 файлов и проверка совместимости.

**Методы:**
- `parse(std::istream&)`: Чтение версии из потока
- `compatibility()`: Возвращает уровень совместимости
- `to_string()`: Строковое представление

**Константы совместимости:**
```cpp
enum Compatibility {
    COMPATIBILITY_DEFAULT = 0,
    COMPATIBILITY_V80316 = 1
};
```

### MessageRegistration System

**Интерфейс MessageRegistrator:**
```cpp
class MessageRegistrator {
public:
    virtual void AddMessage(const String& desc, MessageState state,
                          const String& p1 = "", const String& p2 = "") = 0;
    virtual void Status(const String& message) = 0;
};
```

**Enum MessageState:**
```cpp
enum MessageState {
    Error,
    Warning,
    Info,
    Debug,
    Success
};
```

**Реализации:**
- `ConsoleMessageRegistrator`: Вывод в консоль
- Поддерживает форматированное логирование

## Metadata Module (`src/metadata/`)

### MetadataAnalyzer

**Назначение:**
Анализ метаданных конфигураций, поиск структурных элементов и паттернов.

**Методы:**
- Анализ конфигурационных файлов
- Извлечение зависимостей
- Поиск перечисляемых типов

### RegexRegistry

**Назначение:**
Оптимизированный реестр регулярных выражений с кэшированием.

**Функциональность:**
- `compile(const std::string& pattern)`: Компиляция и кэширование
- `match(const std::string& text)`: Проверка совпадений
- Thread-safe операции

## System Classes (`src/SystemClasses/`)

**Cross-platform subset, имитирующий Delphi/C++Builder:**

### String (Unicode Support)
```cpp
class String {
    std::string data;
public:
    String() = default;
    String(const char* s);
    String(const std::string& s);
    String(const std::wstring& ws); // UTF-16 support
    operator std::string() const;
    operator std::wstring() const;
};
```

### TStream Hierarchy
```
TStream (abstract base)
├── TMemoryStream (in-memory buffer)
├── TFileStream (file operations)
├── TCustomStream
└── THandleStream
```

**Ключевые методы TStream:**
- `Read(void* buffer, size_t size)`
- `Write(const void* buffer, size_t size)`
- `Seek(int64_t offset, SeekOrigin origin)`
- `GetSize()`, `GetPosition()`

## Utils (`src/utils/`)

### ErrorCodes

**Централизованная система ошибок:**
```cpp
enum class ErrorCode {
    SUCCESS = 0,
    FILE_NOT_FOUND = 1,
    INVALID_FORMAT = 2,
    // ...
};

class ErrorInfo {
    ErrorCode code_;
    std::string message_;
    std::string details_;
};
```

**Методы:**
- `to_string()`: Человеко-читаемое описание
- `is_file_error()`: Проверка файловых ошибок

### StringConverters

**Конвертация кодировок:**
- `utf8_to_utf16(const std::string&)`: UTF-8 → UTF-16
- `utf16_to_utf8(const std::u16string&)`: UTF-16 → UTF-8
- Безопасные преобразования без потери данных

## Test Infrastructure (`test/`)

### Основные тестовые файлы:
- `simple_test.cpp`: Базовые функции V8File
- `test_application.cpp`: Компоненты Application layer
- `test_metadata_parser.cpp`: Функции парсинга метаданных

### Структура тестов:
```cpp
int main() {
    // Setup test data
    std::string testDir = "test_data";

    // Execute tests
    auto result = someFunctionToTest();

    // Verify results
    assert(result == expected);

    return 0;
}
```

## Архитектурные Паттерны

### Factory Pattern (CommandFactory)
```cpp
// Создание команд
auto command = factory.createBuildCommand();
// Все зависимости внедрены автоматически
return registry->registerCommand(std::move(command));
```

### Registry Pattern (CommandRegistry)
```cpp
// Регистрация и поиск команд
registry->registerCommand(std::make_unique<BuildCommand>(logger));
auto cmd = registry->getCommand("build"); // Возвращает BuildCommand
```

### Facade Pattern (Application)
```cpp
// Простой интерфейс для сложной системы
int result = app.run(argc, argv);
// Внутри: парсинг → валидация → выполнение → логирование
```

### Strategy Pattern (Command)
```cpp
// Разные алгоритмы выполнения
std::unique_ptr<Command> strategy = std::make_unique<BuildCommand>();
int result = strategy->execute(args);
```

## Заключение

Архитектура V8Unpack демонстрирует современные техники проектирования C++ приложений с акцентом на SOLID принципы, dependency injection и enterprise-grade patterns. Все классы тщательно документированы и следуют единому стилю и паттернам для обеспечения поддерживаемости и расширяемости.
