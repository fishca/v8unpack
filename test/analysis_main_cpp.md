# Анализ структуры main.cpp (Этап 1.1)

## Последовательность выполнения main()

### Flow диаграмма:
```
main() -> Инициализация локали
       -> Разбор аргументов командной строки
       -> get_run_mode() - определение команды
       -> Обработка batch файлов (если указаны)
       -> Подготовка аргументов для handler'а
       -> Выполнение handler'а
       -> Обработка результатов и возврат кода выхода
```

### Основные точки входа:
1. **Одна команда**: `main() -> get_run_mode() -> handler(args)`
2. **Batch режим**: `main() -> get_run_mode() -> process_list() -> handler(args) для каждой строки`

## Анализ глобальных переменных

### Regex переменные (31 переменная):
```cpp
// Генерация паттернов:
std::string StartRegex = R"(\{)";  // Используется: 29 раз
std::string EndRegex   = R"([^}]*\})";  // Используется: 29 раз

// Конкретные regex объекть:
std::regex rx_roles;      // Роли
std::regex rx_comtempl;   // Общие макеты
std::regex rx_commod;     // Общие модули
std::regex rx_http;       // HTTP-сервисы
// ... ~27 других переменных
```

**Использование**: Все regex комбинируются как `StartRegex + GUID + EndRegex` в функции `read_ElementsAllocationTable()`

### Глобальные переменные и их влияние:
1. **StartRegex/EndRegex** - базовые паттерны для всех regex
2. **rx_*** переменные - предкомпилированные regex для поиска типов метаданных в текстовых файлах
3. **GUID константы** - из `guids.h`, используются для конструирования regex

## Диаграмма зависимостей функций

### Основные группы функций:

#### 1. Command Handlers (13 функций):
```
usage() - вывод справки
version() - вывод версии
inflate()/deflate() - сжатие/распаковка
unpack() - распаковка cf/epf
pack() - упаковка cf/epf
parse() - парсинг и сохранение
parsetostring() - парсинг с выводом в консоль
list_files() - вывод списка файлов
process_list() - обработка batch файлов
bat() - генерация bat скриптов
example() - вывод примеров использования
build()/build_nopack() - сборка файлов
save_files() - сохранение файлов
```

#### 2. Infrastructure функции:
```
get_run_mode() - определение режима работы
read_param_file() - чтение batch файла
main() - точка входа
```

#### 3. Metadata анализатор:
```
read_ElementsAllocationTable() - анализ структуры метаданных
OutTree() - вывод дерева метаданных
outtree() - wrapper для OutTree
```

#### 4. Utility функции:
```
wstringToString() / WStringToString() - конвертация строк
removeBOM() / removeBOMutf16() - удаление BOM
utf8_to_string() / from_utf8() - кодировка строк
```

## Связность между функциями (зависимости)

### get_run_mode() вызывает:
- Ничего (чистая функция)

### process_list() вызывает:
- read_param_file()
- get_run_mode() для каждой строки

### main() вызывает:
- get_run_mode()
- read_param_file() (только в batch режиме)
- один из command handler'ов

### read_ElementsAllocationTable() использует:
- Все rx_ regex переменные
- Глобальные GUID константы
- Функции convert'инга строк
- BracketParser

## Анализ паттернов строк

### Args preparation:
```cpp
// Повторяющийся код в большинстве funcs:
// 1. Проверка количества аргументов
// 2. Обработка фильтров (цикл for)
// 3. Вызов основной функции
```

### Regex initialization:
```cpp
// Все regex имеют одинаковый паттерн:
// StartRegex + GUID_xxxx + EndRegex
// Пример: std::regex rx_roles(StartRegex + std::string(GUID_Roles) + EndRegex);
```

### Error handling:
```cpp
// Стандартный паттерн в hex file операциях:
int ret = SomeOperation(...);
if (ret != 0) {
    // logging if logger exists
    return ret;
}
```

## Обнаруженные code smells:

1. **Дублирование кода**: Почти одинаковый пре-процессинг аргументов в каждом handler'е
2. **Глобальное состояние**: 31 глобальная regex переменная
3. **Размазанная логика**: Metadata анализ смешивается с command line обработкой
4. **Неиспользуемая архитектура**: Command pattern не используется в main
5. **Функциональное загрязнение**: main содержит бизнес-логику вместо координации

## Метаданные для рефакторинга

### Код метрики:
- **Строки кода**: ~1700
- **Функции**: ~25
- **Глобальные переменные**: ~32
- **Include'ы**: ~20
- **Using namespace'ы**: 4

### Критичные зависимости:
- Boost::filesystem
- Boost::locale
- Std::regex
- Все V8File функции

### Точки интеграции с новой архитектурой:
1. **CommandFactory** - для создания CommandSystemAdapter
2. **ConsoleMessageRegistrator** - передача в Application
3. **V8File функции** - оборачивание в соответствующие классы

## Следующие шаги подготовки

1. ✅ Создать диаграмму зависимостей (текущий документ)
2. ⏳ Документировать все глобальные переменные (готово)
3. ⏳ Описать последовательность выполнения (готово)
4. ⏳ Выделить точки входа для каждой команды (готово)
5. ⏳ Создать инфраструктуру тестирования
