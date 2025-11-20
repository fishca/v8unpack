# Инструкция по компиляции оптимизированного v8unpack

## 🔧 Подготовка системы

### Ubuntu/Debian/WSL:
```bash
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libboost-all-dev zlib1g-dev
```

### Fedora/RHEL:
```bash
sudo dnf install gcc-c++ cmake
sudo dnf install boost-devel zlib-devel
```

### Arch Linux:
```bash
sudo pacman -S base-devel cmake
sudo pacman -S boost zlib
```

## 🚀 Компиляция

### Метод 1: Автоматическая сборка (рекомендуется)
```bash
cd /home/fishca/v8unpack
./build.sh
```

### Метод 2: Через Makefile
```bash
cd /home/fishca/v8unpack
make clean
make release
```

Исполняемый файл: `bin/Release/v8unpack`

### Метод 3: Через CMake
```bash
cd /home/fishca/v8unpack
mkdir -p build && cd build
cmake ..
cmake --build . --config Release
```

Исполняемый файл: `build/v8unpack`

### Метод 4: Ручная компиляция
```bash
cd /home/fishca/v8unpack

# Создание директорий
mkdir -p obj/Release/src
mkdir -p bin/Release

# Компиляция объектных файлов
g++ -Wall -std=c++14 -O2 -D__LINUX -c src/V8File.cpp -o obj/Release/src/V8File.o
g++ -Wall -std=c++14 -O2 -D__LINUX -c src/utils.cpp -o obj/Release/src/utils.o
g++ -Wall -std=c++14 -O2 -D__LINUX -c src/main.cpp -o obj/Release/src/main.o
g++ -Wall -std=c++14 -O2 -D__LINUX -c src/VersionFile.cpp -o obj/Release/src/VersionFile.o
g++ -Wall -std=c++14 -O2 -D__LINUX -c src/placeholder216.cpp -o obj/Release/src/placeholder216.o

# Линковка
g++ -o bin/Release/v8unpack \
    obj/Release/src/V8File.o \
    obj/Release/src/main.o \
    obj/Release/src/utils.o \
    obj/Release/src/VersionFile.o \
    obj/Release/src/placeholder216.o \
    -lz -lboost_filesystem -lboost_system
```

## ✅ Проверка сборки

```bash
# Проверка версии
./bin/Release/v8unpack -version

# Вывод справки
./bin/Release/v8unpack -help

# Тест распаковки (если есть тестовый файл)
./bin/Release/v8unpack -unpack test.cf test_output
```

## 📦 Установка в систему (опционально)

```bash
sudo make install
# или
sudo cp bin/Release/v8unpack /usr/local/bin/
```

## 🐛 Решение проблем

### Ошибка: "boost/filesystem.hpp: No such file"
```bash
sudo apt-get install libboost-filesystem-dev libboost-system-dev libboost-iostreams-dev
```

### Ошибка: "zlib.h: No such file"
```bash
sudo apt-get install zlib1g-dev
```

### Ошибка: "codecvt: No such file"
Убедитесь, что используется C++14 или новее:
```bash
g++ --version  # должно быть >= 5.0
```

### Линковка не находит библиотеки
Укажите пути явно:
```bash
g++ ... -L/usr/lib/x86_64-linux-gnu -lboost_filesystem -lboost_system -lz
```

## 🎯 Windows (Visual Studio)

### Через CMake GUI:
1. Установите Boost и zlib
2. Откройте CMake GUI
3. Source: `/home/fishca/v8unpack`
4. Build: `/home/fishca/v8unpack/build`
5. Configure → Visual Studio 17 2022
6. Generate
7. Откройте `v8unpack.sln` в Visual Studio
8. Build → Release

### Через командную строку:
```cmd
cmake -G "Visual Studio 17 2022" .
cmake --build . --config Release
```

## 📊 Проверка оптимизаций

После успешной компиляции проверьте производительность:

```bash
# Замер времени распаковки
time ./bin/Release/v8unpack -unpack large_config.cf output_dir

# Сравните с оригинальной версией (если сохранена)
# Ожидаемое ускорение: 30-60% на больших файлах (>100MB)
```

## 📝 Примечания

- **Оптимизированные файлы**: `src/V8File.cpp`, `src/utils.cpp`
- **Резервные копии**: `src/*.backup`
- **Отчет об оптимизациях**: `OPTIMIZATION_REPORT.md`
- **Версия C++**: минимум C++14
- **Boost**: минимум версия 1.53

## 🔙 Откат изменений

Если возникнут проблемы, вернитесь к оригинальной версии:
```bash
cp src/V8File.cpp.backup src/V8File.cpp
cp src/utils.cpp.backup src/utils.cpp
./build.sh
```

---

**Поддержка**: См. `OPTIMIZATION_REPORT.md` для деталей оптимизаций
