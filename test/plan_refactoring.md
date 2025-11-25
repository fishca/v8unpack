# План рефакторинга main.cpp

## Обзор проблемы

Файл `src/main.cpp` содержит ~1700 строк кода с нарушением принципов SOLID и высокой зацепленностью (coupling). Код смешивает несколько ответственностей и имеет высокий технический долг.

## Цели рефакторинга

1. **Улучшить читаемость кода** - разделить ответственность по модулям
2. **Снизить технический долг** - на 70-80%
3. **Повысить тестируемость** - возможность юнит-тестирования отдельных компонентов
4. **Улучшить поддерживаемость** - изменения в одном модуле не затрагивают другие
5. **Обеспечить расширяемость** - легкое добавление новых команд и функциональности

## Текущая структура main.cpp

### Проблемы выявлены:

1. **Смешивание ответственностей:**
   - Парсинг аргументов командной строки
   - Определение всех операций (unpack, pack, deflate, parse)
   - Логика мета-анализа файлов 1C
   - Обработка списков команд
   - Точка входа main()

2. **Глобальные переменные:**
   - ~30 regex выражений в глобальной области
   - Глобальные функции handler'ов

3. **Отсутствие использования существующей архитектуры:**
   - CommandFactory и CommandRegistry не используются
   - Ручной выбор функций через function pointers

## Предлагаемая новая структура

```
src/
├── main.cpp (только точка входа - 50 строк)
├── app/
│   ├── Application.cpp/h (координатор приложения)
│   ├── ArgumentParser.cpp/h (парсинг аргументов)
│   └── ApplicationConfig.cpp/h (конфигурация)
├── commands/
│   ├── legacy/ (старые функции для обратной совместимости)
│   │   ├── LegacyHandlers.cpp/h
│   │   └── LegacyAdapter.cpp/h
│   └── CommandSystemAdapter.cpp/h (адаптер к новой системе команд)
├── metadata/
│   ├── MetadataAnalyzer.cpp/h (анализ мета-данных 1C)
│   ├── RegexRegistry.cpp/h (централизованное управление regex)
│   └── MetadataTypes.h (типы мета-данных)
├── fileops/
│   ├── BatchProcessor.cpp/h (обработка списков команд)
│   ├── FileValidator.cpp/h (валидация файлов)
│   └── OperationResult.cpp/h (результаты операций)
└── utils/
    ├── StringConverters.cpp/h (конвертация строк)
    ├── ErrorCodes.h (коды ошибок)
    └── Logger.cpp/h (расширенное логирование)
```

## Этапы реализации

### Этап 1: Анализ и подготовка (1 неделя)

#### 1.1. Документирование текущей логики
```markdown
- [ ] Создать диаграмму зависимостей между функциями
- [ ] Документировать все глобальные переменные и их использование
- [ ] Описать последовательность выполнения main()
- [ ] Выделить точки входа для каждой команды
```

#### 1.2. Создание инфраструктуры тестирования
```cpp
// test/test_application.cpp - базовый тест структуры
TEST(ApplicationTest, CanCreateApplication) {
    Application app;
    EXPECT_TRUE(app.isValid());
}
```

### Этап 2: Создание классов утилит (1 неделя)

#### 2.1. StringConverters
```cpp
// src/utils/StringConverters.cpp
class StringConverters {
public:
    static std::string fromWstring(const std::wstring& wstr);
    static std::wstring toWstring(const std::string& str);
    static std::string removeBOM(const std::string& str);
    // ...
};
```

#### 2.2. RegexRegistry
```cpp
// src/metadata/RegexRegistry.cpp
class RegexRegistry {
public:
    static std::regex getRegex(const std::string& type);
private:
    static std::map<std::string, std::regex> registry_;
};
```

#### 2.3. ErrorCodes
```cpp
// src/utils/ErrorCodes.h
enum class ErrorCode {
    SUCCESS = 0,
    INVALID_ARGUMENTS = 1,
    FILE_NOT_FOUND = 2,
    // ...
};

class ErrorInfo {
public:
    ErrorCode code;
    std::string message;
    std::string details;
};
```

### Этап 3: Application и ArgumentParser (2 недели)

#### 3.1. ArgumentParser
```cpp
// src/app/ArgumentParser.cpp
class ParsedArgs {
public:
    std::string command;
    std::vector<std::string> args;
    std::map<std::string, std::string> options;
    bool hasListFile = false;
};

class ArgumentParser {
public:
    ParsedArgs parse(int argc, char* argv[]);
private:
    void validateArgs(const ParsedArgs& args);
};
```

#### 3.2. Application
```cpp
// src/app/Application.cpp
class Application {
public:
    Application();
    int run(int argc, char* argv[]);

private:
    std::shared_ptr<ArgumentParser> argParser_;
    std::shared_ptr<CommandRegistry> commandRegistry_;
    std::shared_ptr<MessageRegistrator> logger_;

    ErrorInfo executeCommand(const ParsedArgs& args);
    void handleBatchCommands(const std::string& listFile);
};
```

### Этап 4: Metadata анализатор (1 неделя)

#### 4.1. MetadataAnalyzer
```cpp
// src/metadata/MetadataAnalyzer.cpp
class MetadataAnalyzer {
public:
    struct AnalysisResult {
        std::wstring configVersion;
        std::vector<std::string> foundTypes;
        bool valid = false;
    };

    AnalysisResult analyze(const std::string& filePath);
private:
    std::unique_ptr<RegexRegistry> regexRegistry_;
};
```

### Этап 5: BatchProcessor (1 неделя)

#### 5.1. BatchProcessor
```cpp
// src/fileops/BatchProcessor.cpp
class BatchProcessor {
public:
    struct BatchResult {
        int successCount = 0;
        int errorCount = 0;
        std::vector<ErrorInfo> errors;
    };

    BatchResult processListFile(const std::string& filePath,
                               std::function<ErrorInfo(const std::vector<std::string>&)> processor);
};
```

### Этап 6: CommandSystemAdapter (2 недели)

#### 6.1. Адаптер для существующих функций
```cpp
// src/commands/CommandSystemAdapter.cpp
class CommandSystemAdapter {
public:
    void registerLegacyCommands(CommandRegistry& registry);

    // Адаптеры для старых функций
    static std::unique_ptr<Command> createLegacyUnpackCommand();
    static std::unique_ptr<Command> createLegacyPackCommand();
    // ...
};
```

### Этап 7: Миграция и рефакторинг main.cpp (2 недели)

#### 7.1. Пошаговая миграция
```cpp
// Шаг 1: Выделение Application класса
int main(int argc, char* argv[]) {
    try {
        Application app;
        return app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

// Шаг 2: Перенос функций в соответствующие модули
// Шаг 3: Постепенное уменьшение main.cpp
// Шаг 4: Удаление устаревшего кода с сохранением совместимости
```

### Этап 8: Тестирование и валидация (2 недели)

#### 8.1. Модульное тестирование
```cpp
// test/test_argument_parser.cpp
TEST(ArgumentParserTest, ParseSimpleCommand) {
    const char* argv[] = {"v8unpack", "-unpack", "test.cf", "output/"};
    ArgumentParser parser;
    auto result = parser.parse(4, const_cast<char**>(argv));

    EXPECT_EQ(result.command, "unpack");
    EXPECT_EQ(result.args.size(), 2);
}

// test/test_application.cpp
TEST(ApplicationTest, RunUnpackCommand) {
    Application app;
    const char* argv[] = {"v8unpack", "-unpack", "test.cf", "output/"};

    int result = app.run(4, const_cast<char**>(argv));
    EXPECT_EQ(result, 0);
}
```

#### 8.2. Интеграционное тестирование
```bash
# test/run_refactoring_tests.sh
./test_argument_parser
./test_application
./test_metadata_analyzer
# Сравнение результатов с эталонными
```

### Этап 9: Оптимизация и финализация (1 неделя)

#### 9.1. Производительность
```cpp
// Замер производительности новых компонентов
BENCHMARK(BenchmarkMetadataAnalysis) {
    MetadataAnalyzer analyzer;
    // ...
}
```

#### 9.2. Память
- Проверка на утечки памяти
- Оптимизация использования памяти

## Риски и Mitigation

### Риски:
1. **Регрессионные ошибки:** Некоторые edge case может быть упущен
2. **Проблемы совместимости:** Изменения могут сломать существующие скрипты
3. **Производительность:** Увеличение overhead от дополнительной абстракции

### Mitigation:
1. **Пошаговая миграция** с сохранением старого кода
2. **Обширное тестирование** на всех существующих сценариях
3. **Feature flags** для переключения между старой и новой реализацией
4. **Профилирование** производительности на каждом этапе

## Критерии успеха

1. **Функциональная совместимость:** Все существующие команды работают
2. **Код метрики:**
   - Строки кода в main.cpp: 1700+ → 50-70
   - Уровень покрытия тестами: минимум 80%
   - Цикломатическая сложность: снижение на 60%
3. **Производительность:** Не ухудшение более чем на 5%
4. **Поддерживаемость:** Время на добавление новой команды -30%

## Ресурсы и timeline

### Общий timeline: 11 недель
- Подготовка: 1 неделя
- Утилиты: 1 неделя
- Application: 2 недели
- Metadata: 1 неделя
- BatchProcessor: 1 неделя
- CommandAdapter: 2 недели
- Миграция main: 2 недели
- Тестирование: 2 недели
- Финализация: 1 неделя

### Команда: 2-3 разработчика
1. **Архитектор:** Разработка общей архитектуры и надзор
2. **Разработчик A:** Реализация модулей Application, MetadataAnalyzer, utils
3. **Разработчик B:** Реализация CommandSystemAdapter, тестирование
4. **QA инженер:** Написание и выполнение тестов

## Следующие шаги для запуска

1. ✅ Анализ завершен
2. ⏳ Создать подробные спецификации для каждого класса
3. ⏳ Начать с утилитарных классов (StringConverters, ErrorCodes)
4. ⏳ Разработать тесты параллельно с кодом
