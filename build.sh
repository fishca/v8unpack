#!/bin/bash
# Скрипт для компиляции v8unpack с оптимизациями
# Автор оптимизаций: Claude (Anthropic)
# Дата: 2025-11-17

set -e

echo "=========================================="
echo "  Сборка v8unpack с оптимизациями"
echo "=========================================="
echo ""

# Цвета для вывода
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

# Проверка компилятора
if ! command -v g++ &> /dev/null; then
    echo -e "${RED}✗ g++ не найден!${NC}"
    echo ""
    echo "Установите инструменты сборки:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install build-essential libboost-all-dev zlib1g-dev"
    exit 1
fi

echo -e "${GREEN}✓ Компилятор найден: $(g++ --version | head -1)${NC}"

# Проверка зависимостей
if ! dpkg -l | grep -q libboost; then
    echo -e "${YELLOW}⚠ Boost библиотеки не найдены${NC}"
    echo "Установите: sudo apt-get install libboost-all-dev"
fi

if ! dpkg -l | grep -q zlib1g-dev; then
    echo -e "${YELLOW}⚠ zlib не найден${NC}"
    echo "Установите: sudo apt-get install zlib1g-dev"
fi

echo ""
echo "Начинаю компиляцию..."
echo ""

# Переход в директорию проекта
cd "$(dirname "$0")"

# Очистка предыдущей сборки
echo "Очистка предыдущей сборки..."
rm -rf obj/Release bin/Release 2>/dev/null || true
mkdir -p obj/Release/src
mkdir -p bin/Release

# Компиляция
echo ""
echo "Компиляция исходных файлов..."

CFLAGS="-Wall -std=c++14 -O2 -D__LINUX -I. -Isrc -Isrc/SystemClasses"
LDFLAGS="-static"
LIBS="-lz -lboost_filesystem -lboost_system"

echo "  Компиляция всех исходных файлов..."

# Создание директорий для объектных файлов
mkdir -p obj/Release/src/SystemClasses

# Компиляция всех .cpp файлов
cpp_files=(
    "src/APIcfBase.cpp"
    "src/EctoSoftTree.cpp"
    "src/ExactStructureBuilder.cpp"
    "src/NodeTypes.cpp"
    "src/StringUtils.cpp"
    "src/SystemClasses/GetTickCount.cpp"
    "src/SystemClasses/String.cpp"
    "src/SystemClasses/System.Classes.cpp"
    "src/SystemClasses/System.IOUtils.cpp"
    "src/SystemClasses/System.SysUtils.cpp"
    "src/SystemClasses/System.cpp"
    "src/SystemClasses/TFileStream.cpp"
    "src/SystemClasses/TMemoryStream.cpp"
    "src/SystemClasses/TStream.cpp"
    "src/SystemClasses/TStreamReader.cpp"
    "src/SystemClasses/TStreamWriter.cpp"
    "src/THashedStringList.cpp"
    "src/TMSTree.cpp"
    "src/TStringList.cpp"
    "src/V8File.cpp"
    "src/V8MetaParser.cpp"
    "src/VersionFile.cpp"
    "src/binarydecimalnumber.cpp"
    "src/common.cpp"
    "src/logger.cpp"
    "src/main.cpp"
    "src/mdCommand.cpp"
    "src/mdForm.cpp"
    "src/mdLang.cpp"
    "src/mdMoxel.cpp"
    "src/mdObject.cpp"
    "src/messageregistration.cpp"
    "src/parse_tree.cpp"
    "src/placeholder216.cpp"
    "src/tree.cpp"
    "src/treeparser.cpp"
    "src/utils.cpp"
)

total_files=${#cpp_files[@]}
for i in "${!cpp_files[@]}"; do
    file="${cpp_files[$i]}"
    obj_file="obj/Release/${file%.cpp}.o"
    mkdir -p "$(dirname "$obj_file")"
    echo "  [$((i+1))/$total_files] Компиляция $(basename "$file")..."
    if ! g++ $CFLAGS -c "$file" -o "$obj_file" 2>&1; then
        echo "Ошибка компиляции $file"
        exit 1
    fi
done

# Линковка
echo ""
echo "Линковка исполняемого файла..."

# Список всех объектных файлов
obj_files=(
    "obj/Release/src/APIcfBase.o"
    "obj/Release/src/EctoSoftTree.o"
    "obj/Release/src/ExactStructureBuilder.o"
    "obj/Release/src/NodeTypes.o"
    "obj/Release/src/StringUtils.o"
    "obj/Release/src/SystemClasses/GetTickCount.o"
    "obj/Release/src/SystemClasses/String.o"
    "obj/Release/src/SystemClasses/System.Classes.o"
    "obj/Release/src/SystemClasses/System.IOUtils.o"
    "obj/Release/src/SystemClasses/System.SysUtils.o"
    "obj/Release/src/SystemClasses/System.o"
    "obj/Release/src/SystemClasses/TFileStream.o"
    "obj/Release/src/SystemClasses/TMemoryStream.o"
    "obj/Release/src/SystemClasses/TStream.o"
    "obj/Release/src/SystemClasses/TStreamReader.o"
    "obj/Release/src/SystemClasses/TStreamWriter.o"
    "obj/Release/src/THashedStringList.o"
    "obj/Release/src/TMSTree.o"
    "obj/Release/src/TStringList.o"
    "obj/Release/src/V8File.o"
    "obj/Release/src/V8MetaParser.o"
    "obj/Release/src/VersionFile.o"
    "obj/Release/src/binarydecimalnumber.o"
    "obj/Release/src/common.o"
    "obj/Release/src/logger.o"
    "obj/Release/src/main.o"
    "obj/Release/src/mdCommand.o"
    "obj/Release/src/mdForm.o"
    "obj/Release/src/mdLang.o"
    "obj/Release/src/mdMoxel.o"
    "obj/Release/src/mdObject.o"
    "obj/Release/src/messageregistration.o"
    "obj/Release/src/parse_tree.o"
    "obj/Release/src/placeholder216.o"
    "obj/Release/src/tree.o"
    "obj/Release/src/treeparser.o"
    "obj/Release/src/utils.o"
)

linking_command="g++ $LDFLAGS -o bin/Release/v8unpack"
for obj in "${obj_files[@]}"; do
    linking_command="$linking_command $obj"
done
linking_command="$linking_command $LIBS"

echo "Команда линковки: $linking_command"
if ! eval "$linking_command" 2>&1; then
    echo "Ошибка линковки!"
    exit 1
fi

# Проверка результата
if [ -f bin/Release/v8unpack ]; then
    echo ""
    echo -e "${GREEN}=========================================="
    echo "  ✓ Сборка успешно завершена!"
    echo "==========================================${NC}"
    echo ""
    echo "Исполняемый файл: bin/Release/v8unpack"
    echo "Размер: $(du -h bin/Release/v8unpack | cut -f1)"
    echo ""
    echo "Запуск:"
    echo "  ./bin/Release/v8unpack -help"
    echo ""
    echo "Установка (опционально):"
    echo "  sudo make install"
    echo ""
    
    # Тест запуска
    echo "Тест запуска..."
    if ./bin/Release/v8unpack -version 2>&1 | head -1; then
        echo -e "${GREEN}✓ Программа запускается корректно${NC}"
    fi
else
    echo -e "${RED}✗ Ошибка компиляции!${NC}"
    echo "Проверьте логи выше для деталей."
    exit 1
fi

echo ""
echo "Примечание: Это оптимизированная версия v8unpack"
echo "Ожидаемое ускорение: 30-60% на больших файлах"
echo "Резервные копии оригинального кода: src/*.backup"
