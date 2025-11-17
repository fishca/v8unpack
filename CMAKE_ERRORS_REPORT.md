# Отчёт об ошибках в CMakeLists.txt

Дата проверки: 2025-11-17

## Критические ошибки

### 1. Отсутствуют файлы SystemClasses в V8UNPACK_SOURCES
**Расположение**: строки 18-43
**Проблема**: В списке исходных файлов не указаны 11 файлов из каталога `src/SystemClasses/`, которые необходимы для успешной компиляции.

**Отсутствующие файлы**:
- src/SystemClasses/String.cpp
- src/SystemClasses/System.Classes.cpp
- src/SystemClasses/System.SysUtils.cpp
- src/SystemClasses/System.IOUtils.cpp
- src/SystemClasses/System.cpp
- src/SystemClasses/TStream.cpp
- src/SystemClasses/TMemoryStream.cpp
- src/SystemClasses/TFileStream.cpp
- src/SystemClasses/TStreamReader.cpp
- src/SystemClasses/TStreamWriter.cpp
- src/SystemClasses/GetTickCount.cpp

**Последствия**: При сборке с помощью CMake возникнут ошибки линковки типа "undefined reference" для классов `System::String`, `TStream`, `TMemoryStream` и т.д.

**Решение**: Добавить все указанные файлы в переменную `V8UNPACK_SOURCES`.

---

## Ошибки средней критичности

### 2. Неправильное имя файла в V8UNPACK_HEADERS
**Расположение**: строка 56
**Проблема**: Указан файл `src/nodetypes.h`, но в файловой системе (Linux с чувствительностью к регистру) этот файл называется `src/NodeTypes.h` (с заглавной N).

**Текущее**:
```cmake
src/nodetypes.h
```

**Должно быть**:
```cmake
src/NodeTypes.h
```

**Последствия**: При компиляции в Linux возникнет ошибка "No such file or directory".

---

### 3. Файл .cpp в списке заголовочных файлов
**Расположение**: строка 63
**Проблема**: В переменной `V8UNPACK_HEADERS` (заголовочные файлы) указан файл `src/utils.cpp`, который является исходным файлом, а не заголовочным.

**Текущее**:
```cmake
src/utils.cpp
```

**Должно быть**: Удалить эту строку (или заменить на `src/utils.h`, если такой файл существует).

**Последствия**: Технически не критично для CMake, но нарушает логику и может вызвать путаницу.

---

## Предупреждения и замечания

### 4. Отсутствует директива include_directories для локальных заголовков
**Расположение**: после строки 107
**Проблема**: Не указаны пути к локальным заголовочным файлам проекта.

**Рекомендация**: Добавить:
```cmake
include_directories (src src/SystemClasses)
```

**Последствия**: При сборке возможны ошибки типа "file not found" для заголовков из SystemClasses.

---

### 5. Жёстко заданные пути для Windows
**Расположение**: строки 97-98
**Проблема**: Указаны абсолютные пути к библиотекам Boost, специфичные для Windows:

```cmake
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/Libraries/boost_1_60_0")
SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Libraries/boost_1_60_0/lib32-msvc-15.0")
```

**Рекомендация**: Обернуть эти строки в условный блок:
```cmake
if(WIN32)
	SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "C:/Libraries/boost_1_60_0")
	SET(CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH} "C:/Libraries/boost_1_60_0/lib32-msvc-15.0")
endif()
```

**Последствия**: В Linux эти пути игнорируются, но засоряют конфигурацию.

---

### 6. Отсутствует определение __LINUX для Linux-сборки
**Расположение**: после строки 16
**Проблема**: В коде используется препроцессорная директива `#ifdef __LINUX`, но CMakeLists.txt не определяет этот макрос для Linux.

**Рекомендация**: Добавить:
```cmake
if(UNIX)
	add_definitions(-D__LINUX)
endif()
```

**Последствия**: Код, зависящий от платформы (например, функции преобразования строк в `main.cpp` и `V8File.cpp`), может работать некорректно.

---

## Исправленная версия

Исправленная версия файла сохранена в: **CMakeLists.txt.fixed**

### Основные изменения:

1. ✅ Добавлены все 11 файлов из SystemClasses в V8UNPACK_SOURCES
2. ✅ Исправлено имя `src/nodetypes.h` → `src/NodeTypes.h`
3. ✅ Удалён `src/utils.cpp` из V8UNPACK_HEADERS
4. ✅ Добавлен `include_directories (src src/SystemClasses)`
5. ✅ Жёстко заданные пути Boost обёрнуты в `if(WIN32)`
6. ✅ Добавлено определение `-D__LINUX` для Unix-систем

### Команды для применения исправлений:

```bash
# Создать резервную копию
cp CMakeLists.txt CMakeLists.txt.backup

# Применить исправления
cp CMakeLists.txt.fixed CMakeLists.txt

# Пересобрать проект (если используется CMake)
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

---

## Проверка корректности сборки

После применения исправлений проект **должен успешно компилироваться** с помощью CMake как в Windows, так и в Linux.

Текущий Makefile уже исправлен и работает корректно. CMakeLists.txt требует обновления для соответствия текущей структуре проекта.
