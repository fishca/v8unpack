# Парсинг метаданных конфигурации 1С

## Описание решения

Создан класс `ConfigMetadataParser` для парсинга структуры конфигурации 1С и определения имён файлов по GUID.

### Файл: [src/ConfigMetadataParser.h](src/ConfigMetadataParser.h)

## Возможности

1. **Парсинг структуры конфигурации** из файлов формата `{guid,count,item_guid1,...}`
2. **Маппинг GUID → имена метаданных** (используя существующие константы из `guids.h`)
3. **Генерация имён файлов** по типу метаданных и GUID объекта
4. **Вывод сводки** по всем метаданным в конфигурации

## Структура данных

### MetadataInfo
```cpp
struct MetadataInfo {
    std::string guid;              // GUID типа метаданных
    std::wstring type_name;        // Имя типа (например, "Справочники")
    int count;                     // Количество объектов
    std::vector<std::string> items; // Список GUID объектов
};
```

## Примеры использования

### Пример 1: Базовое использование

```cpp
#include "ConfigMetadataParser.h"
#include <fstream>
#include <iostream>

using namespace v8unpack;

int main() {
    // Создаём парсер
    ConfigMetadataParser parser;

    // Читаем файл конфигурации
    std::ifstream file("d5e20966-24a3-4184-a1b0-e5aa4773c2b8.txt");
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());

    // Парсим содержимое
    parser.parseConfigFile(content);

    // Выводим сводку
    parser.printSummary();

    return 0;
}
```

**Вывод:**
```
=== Сводка по метаданным конфигурации ===

Языки (9cd510ce-abfc-11d4-9434-004095e12fc7): 1 объект(ов)
  - 154c4235-35f5-42c3-a0d5-07b9ea861f14

Справочники (cf4abea6-37b2-11d4-940f-008048da11f9): 1 объект(ов)
  - 36b34b4a-30fc-40f3-be9d-59e7d39f8e95
```

### Пример 2: Получение имени типа метаданных

```cpp
ConfigMetadataParser parser;

// По GUID типа метаданных получаем имя
std::wstring name = parser.getMetadataTypeName(GUID_Catalogs);
// Результат: "Справочники"

std::wstring lang_name = parser.getMetadataTypeName(GUID_Languages);
// Результат: "Языки"
```

### Пример 3: Генерация имени файла

```cpp
ConfigMetadataParser parser;
parser.parseConfigFile(content);

// Генерируем имя файла для справочника
std::string filename = parser.generateFileName(
    GUID_Catalogs,  // Тип: Справочники
    "36b34b4a-30fc-40f3-be9d-59e7d39f8e95"  // GUID объекта
);
// Результат: "Справочники_36b34b4a-30fc-40f3-be9d-59e7d39f8e95"

// Генерируем имя файла для языка
std::string lang_file = parser.generateFileName(
    GUID_Languages,  // Тип: Языки
    "154c4235-35f5-42c3-a0d5-07b9ea861f14"  // GUID объекта
);
// Результат: "Языки_154c4235-35f5-42c3-a0d5-07b9ea861f14"
```

### Пример 4: Обработка всех метаданных

```cpp
ConfigMetadataParser parser;
parser.parseConfigFile(content);

// Получаем все метаданные
const auto& all_metadata = parser.getAllMetadata();

// Обрабатываем каждый тип метаданных
for (const auto& [type_guid, info] : all_metadata) {
    if (info.count > 0) {
        std::cout << "Тип: " << info.type_name
                  << ", Количество: " << info.count << std::endl;

        // Обрабатываем каждый объект
        for (const auto& item_guid : info.items) {
            std::string filename = parser.generateFileName(type_guid, item_guid);
            std::cout << "  Файл: " << filename << std::endl;
        }
    }
}
```

### Пример 5: Интеграция с существующим кодом v8unpack

```cpp
#include "V8File.h"
#include "ConfigMetadataParser.h"

int processConfiguration(const std::string& cf_file, const std::string& output_dir) {
    using namespace v8unpack;

    // 1. Распаковываем CF файл
    CV8File v8file;
    v8file.LoadFile(cf_file);

    // 2. Получаем содержимое файла root
    std::string root_content = getDataFromFile1C(cf_file, "root");

    // 3. Парсим root и получаем GUID конфигурации
    // (используя BracketParser или regex)
    std::regex root_guid_pattern(R"(\{2,([0-9a-fA-F-]+))");
    std::smatch match;
    std::string config_guid;
    if (std::regex_search(root_content, match, root_guid_pattern)) {
        config_guid = match[1].str();
    }

    // 4. Получаем содержимое файла конфигурации
    std::string config_content = getDataFromFile1C(cf_file, config_guid);

    // 5. Парсим метаданные
    ConfigMetadataParser parser;
    parser.parseConfigFile(config_content);

    // 6. Обрабатываем каждый тип метаданных
    const auto& all_metadata = parser.getAllMetadata();
    for (const auto& [type_guid, info] : all_metadata) {
        for (const auto& item_guid : info.items) {
            // Генерируем читаемое имя файла
            std::string filename = parser.generateFileName(type_guid, item_guid);

            // Получаем данные объекта
            std::string object_data = getDataFromFile1C(cf_file, item_guid);

            // Сохраняем с читаемым именем
            std::string output_path = output_dir + "/" + filename + ".txt";
            std::ofstream out(output_path);
            out << object_data;
            out.close();

            std::cout << "Создан файл: " << filename << std::endl;
        }
    }

    return 0;
}
```

## Формат входных данных

Парсер ожидает текст в формате:

```
{guid_type,count,item_guid1,item_guid2,...}
```

### Примеры входных строк:

```cpp
// Языки: 1 объект
"{9cd510ce-abfc-11d4-9434-004095e12fc7,1,154c4235-35f5-42c3-a0d5-07b9ea861f14}"

// Справочники: 1 объект
"{cf4abea6-37b2-11d4-940f-008048da11f9,1,36b34b4a-30fc-40f3-be9d-59e7d39f8e95}"

// Роли: 0 объектов
"{09736b02-9cac-4e3f-b4f7-d3e9576ab948,0}"

// Документы: 3 объекта
"{061d872a-5787-460e-95ac-ed74ea3a3e84,3,guid1,guid2,guid3}"
```

## Поддерживаемые типы метаданных

Парсер поддерживает все типы метаданных, определённые в [src/guids.h](src/guids.h):

### Общие метаданные:
- Подсистемы (Subsystems)
- Общие модули (CommonModules)
- Параметры сеанса (SessionParameters)
- Роли (Roles)
- Общие реквизиты (CommonAttributes)
- Планы обмена (ExchangePlans)
- И другие...

### Основные объекты:
- **Справочники** (Catalogs) - `cf4abea6-37b2-11d4-940f-008048da11f9`
- **Документы** (Documents) - `061d872a-5787-460e-95ac-ed74ea3a3e84`
- **Константы** (Constants) - `0195e80c-b157-11d4-9435-004095e12fc7`
- **Отчёты** (Reports) - `631b75a0-29e2-11d6-a3c7-0050bae0a776`
- **Обработки** (DataProcessors) - `bf845118-327b-4682-b5c6-285d2a0eb296`
- **Перечисления** (Enums) - `f6a80749-5ad7-400b-8519-39dc5dff2542`
- **Языки** (Languages) - `9cd510ce-abfc-11d4-9434-004095e12fc7`
- И другие (всего 40+ типов)

## Алгоритм работы

1. **Инициализация**: Создаётся маппинг GUID → имя метаданных из констант `guids.h`

2. **Парсинг**:
   - Читается содержимое файла конфигурации
   - Для каждой строки формата `{guid,count,...}`:
     - Извлекаются все GUID с помощью regex
     - Первый GUID - тип метаданных
     - Следующее число - количество объектов
     - Остальные GUID - конкретные объекты

3. **Генерация имён**:
   - По GUID типа метаданных получается читаемое имя
   - Формируется имя файла: `ТипМетаданных_ObjectGUID`

## Преимущества решения

1. ✅ **Использует существующие константы** из `guids.h` - нет дублирования
2. ✅ **Простой API** - всего несколько методов для работы
3. ✅ **Поддержка всех типов** метаданных 1С (40+ типов)
4. ✅ **Читаемые имена файлов** вместо только GUID
5. ✅ **Гибкость** - можно получить как полную информацию, так и только имя
6. ✅ **Header-only** - весь код в одном заголовочном файле

## Интеграция в проект

Для использования парсера нужно:

1. Включить заголовочный файл:
```cpp
#include "ConfigMetadataParser.h"
```

2. Добавить в CMakeLists.txt (уже есть в списке заголовков):
```cmake
src/ConfigMetadataParser.h
```

3. Использовать в коде (см. примеры выше)

## Дальнейшие улучшения

Возможные расширения функциональности:

1. **Парсинг имён объектов** - извлечение реальных имён справочников/документов из их описаний
2. **Иерархия метаданных** - построение дерева зависимостей
3. **Экспорт в JSON/XML** - сохранение структуры конфигурации
4. **Валидация** - проверка целостности метаданных
5. **Кэширование** - сохранение распарсенной структуры

## Тестирование

Для тестирования парсера можно использовать предоставленный пример файла:

```bash
# Создаём тестовый файл
cat > test_config.txt << 'EOF'
{9cd510ce-abfc-11d4-9434-004095e12fc7,1,154c4235-35f5-42c3-a0d5-07b9ea861f14}
{cf4abea6-37b2-11d4-940f-008048da11f9,1,36b34b4a-30fc-40f3-be9d-59e7d39f8e95}
EOF

# Компилируем и запускаем тест
g++ -std=c++14 -I src test_parser.cpp -o test_parser
./test_parser test_config.txt
```

## Контакты и вопросы

Для вопросов по использованию парсера обращайтесь к документации проекта или создавайте issue в репозитории.
