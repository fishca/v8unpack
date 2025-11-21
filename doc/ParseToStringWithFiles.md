# Логика работы функции ParseToStringWithFiles

## Общее описание

Функция `ParseToStringWithFiles` находится в файле `src/V8MetaParser.cpp` и предназначена для обработки строкового представления распакованной конфигурации 1C:Предприятие (полученного из `ParseToString`) и создания организованной файловой структуры с файлами, сгруппированными по типам метаданных.

## Входные параметры

- `config_string`: Строковое представление распарсенной конфигурации, полученное от `ParseToString`. Оно состоит из секций, разделенных разделителями в формате `--- ИМЯ_СЕКЦИИ ---`, за которыми следуют данные секции.
- `dirname`: Абсолютный путь к каталогу, куда будет произведена распаковка. В этом каталоге создаются подкаталоги для разных типов объектов.

## Основная логика

### 1. Подготовка директорий

```cpp
fs::path base_dir(dirname);
if (!ensure_directory(base_dir)) {
    std::cerr << "ParseToStringWithFiles: Failed to create base directory: " << dirname << std::endl;
    return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
}

fs::path config_dir = base_dir / "Конфигурация";
if (!ensure_directory(config_dir)) {
    std::cerr << "ParseToStringWithFiles: Failed to create configuration directory: " << config_dir.string() << std::endl;
    return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
}
```

- Создается базовый каталог `dirname`.
- Внутри него создается подкаталог `Конфигурация` для хранения основных конфигурационных файлов (root, version, versions, metadata).

### 2. Маппинг типов метаданных

Определяется `unordered_map<std::string, std::string> guid_to_type`, который сопоставляет GUIDы типов метаданных с их русскими названиями:

```cpp
std::unordered_map<std::string, std::string> guid_to_type = {
    // Подсистемы
    {"37f2fa9a-b276-11d4-9435-004095e12fc7", "Подсистемы"},
    // Различные типы метаданных (модули, формы, макеты, роли и т.д.)
    // ...
};
```

Этот маппинг используется для определения категории объекта по GUIDам в его данных или имени файла.

### 3. Разбор строкового представления конфигурации

```cpp
std::istringstream stream(config_string);
std::string line;
std::string current_section;
std::string current_data;
int file_count = 0;
```

- Инициализируется счетчик файлов `file_count`.
- `config_string` разбивается на строки с помощью `getline`.
- Процесс обработки ведется по секциям, разделенным маркерами `--- `.

### 4. Обработка секций

Для каждой строки проверяется:

```cpp
if (line.find("--- ") == 0 && line.find(" ---") != std::string::npos) {
    // Обработка завершения предыдущей секции
    if (!current_section.empty()) {
        // Логика сохранения данной секции
    }

    // Начало новой секции
    size_t start_pos = line.find("--- ") + 4;
    size_t end_pos = line.find(" ---");
    if (end_pos > start_pos) {
        current_section = line.substr(start_pos, end_pos - start_pos);
    }
    current_data.clear();
} else {
    // Добавление данных к текущей секции
    if (!current_data.empty()) {
        current_data += "\n";
    }
    current_data += line;
}
```

#### 4.1 Обработка конфигурационных файлов

```cpp
if (current_section.find("root") != std::string::npos ||
    current_section == "version" ||
    current_section == "versions" ||
    current_section == "metadata") {
    logger.log("Found config section: " + current_section);

    // Сохранение в подкаталог Конфигурация
    std::string filename = current_section;
    if (filename.find("root") != std::string::npos) filename = "root";

    fs::path file_path = config_dir / filename;
    std::ofstream out_file(file_path.string(), std::ios::binary);
    if (out_file) {
        out_file.write(current_data.c_str(), current_data.size());
        out_file.close();
        file_count++;
        logger.log("Saved config file: " + file_path.string());
    }
}
```

Конфигурационные файлы сохраняются напрямую в `Конфигурация/` без дополнительной обработки.

#### 4.2 Анализ корневого файла для построения маппинга объектов

При обработке файла `root` производится:

- Поиск GUIDа конфигурации: `std::regex_search(current_data, match, config_guid_pattern)`
- Парсинг структуры файла с помощью `BracketParser`
- Анализ секции метаданных для построения маппинга `type_to_objects`: GUID типа -> список GUIDов объектов

#### 4.3 Обработка объектов метаданных

Для неконфигурационных секций:

```cpp
// Определение типа метаданных
std::string metadata_type = "Неизвестные_объекты";

// Попытка найти тип по GUID в имени файла
std::string potential_guid = current_section;
std::transform(potential_guid.begin(), potential_guid.end(), potential_guid.begin(), ::tolower);

// Если имя файла является GUID, найти его тип
if (regex_match(potential_guid, is_guid_pattern)) {
    for (const auto& type_pair : type_to_objects) {
        auto obj_it = std::find(type_pair.second.begin(), type_pair.second.end(), potential_guid);
        if (obj_it != type_pair.second.end()) {
            metadata_type = type_pair.first;
            break;
        }
    }
}

// Fallback: поиск GUIDов в данных
if (metadata_type == "Неизвестные_объекты") {
    auto guids = find_guids(current_data);
    for (const auto& guid : guids) {
        auto type_it = guid_to_type.find(guid);
        if (type_it != guid_to_type.end()) {
            metadata_type = type_it->second;
            break;
        }
    }
}

// Создание подкаталога для типа
fs::path type_dir = base_dir / metadata_type;
if (!ensure_directory(type_dir)) {
    std::cerr << "ParseToStringWithFiles: Failed to create type directory: " << type_dir.string() << std::endl;
    continue;
}
```

##### 4.3.1 Сохранение или распаковка объекта

```cpp
// Подготовка имени файла
std::string filename = current_section;
// Удаление недопустимых символов
std::replace(filename.begin(), filename.end(), '"', '_');
// ...

// Проверка, является ли объект V8-контейнером
std::istringstream data_stream(current_data);
if (IsV8File(data_stream)) {
    // Распаковка в подкаталог
    fs::path sub_dir = type_dir / current_section;
    if (!ensure_directory(sub_dir)) {
        std::cerr << "ParseToStringWithFiles: Failed to create subdirectory: " << sub_dir.string() << std::endl;
    } else {
        data_stream.seekg(0);
        int unpack_result = RecursiveUnpack(sub_dir.string(), data_stream, {}, true, false);
        if (unpack_result != V8UNPACK_OK) {
            // Fallback: сохранение как файл
            fs::path file_path = type_dir / filename;
            // ...
        } else {
            file_count++;
            logger.log("Unpacked container to directory: " + sub_dir.string());
        }
    }
} else {
    // Сохранение как обычный файл
    fs::path file_path = type_dir / filename;
    std::ofstream out_file(file_path.string(), std::ios::binary);
    // ...
    file_count++;
}
```

### 5. Завершение обработки

```cpp
// Обработка последней секции аналогично предыдущим
if (!current_section.empty()) {
    // Та же логика, что и для предыдущих секций
}

logger.log("ParseToStringWithFiles completed. Saved " + std::to_string(file_count) + " files.");
```

## Особенности и нюансы

1. **Разделители секций**: Конфигурация разделяется на секции по маркерам `--- ИМЯ ---`.

2. **Двоичные данные**: Функция работает с текстовым представлением, но сохраняет данные в бинарном формате.

3. **Вложенные контейнеры**: Если объект является V8-контейнером, он распаковывается в поддиректорию.

4. **Маппинг типов**: Анализ GUIDов позволяет автоматически категоризировать объекты по типам метаданных.

5. **Fallback механизмы**: При невозможности распаковки контейнера объект сохраняется как файл.

6. **Дублирование**:
   - Конфигурационные файлы попадают в два места: в корень каталога и в `Конфигурация/`
   - Это связано с тем, что они обрабатываются как отдельные секции в результата ParserToString и дополнительно сохраняются для удобства обращения.

## Возвращаемые значения

- `V8UNPACK_OK` в случае успеха
- `V8UNPACK_ERROR_CREATING_OUTPUT_FILE` при ошибках создания директорий

Функция обеспечивает создание хорошо организованной файловой структуры, где файлы сгруппированы по типам метаданных 1C:Предприятия, что значительно облегчает анализ и работу с распакованной конфигурацией.
