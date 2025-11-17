/**
 * @file test_metadata_parser.cpp
 * @brief Тестовое приложение для демонстрации работы ConfigMetadataParser
 *
 * Компиляция:
 *   g++ -std=c++14 -I src test_metadata_parser.cpp -o test_metadata_parser
 *
 * Использование:
 *   ./test_metadata_parser <config_file.txt>
 */

#include "src/ConfigMetadataParser.h"
#include <fstream>
#include <iostream>
#include <locale>
#include <codecvt>

using namespace v8unpack;

int main(int argc, char* argv[]) {
    // Устанавливаем UTF-8 локаль для корректного вывода кириллицы
    std::locale::global(std::locale(""));
    std::wcout.imbue(std::locale());

    std::cout << "=== Тестирование ConfigMetadataParser ===" << std::endl;
    std::cout << std::endl;

    // Пример данных из документа
    std::string example_data = R"(
{2,
{d5e20966-24a3-4184-a1b0-e5aa4773c2b8},7,
{9cd510cd-abfc-11d4-9434-004095e12fc7,
{1,
{67,
{0,
{3,
{1,0,a3052b8e-768b-49d6-a4da-1c60df359621}
}
{9cd510ce-abfc-11d4-9434-004095e12fc7,1,154c4235-35f5-42c3-a0d5-07b9ea861f14},
{cf4abea6-37b2-11d4-940f-008048da11f9,1,36b34b4a-30fc-40f3-be9d-59e7d39f8e95},
{09736b02-9cac-4e3f-b4f7-d3e9576ab948,0},
{0c89c792-16c3-11d5-b96b-0050bae0a95d,0},
{0fe48980-252d-11d6-a3c7-0050bae0a776,0},
{0195e80c-b157-11d4-9435-004095e12fc7,0},
{061d872a-5787-460e-95ac-ed74ea3a3e84,0},
{07ee8426-87f1-11d5-b99c-0050bae0a95d,0},
{13134201-f60b-11d5-a3c7-0050bae0a776,0},
{631b75a0-29e2-11d6-a3c7-0050bae0a776,0},
{bf845118-327b-4682-b5c6-285d2a0eb296,0},
{f6a80749-5ad7-400b-8519-39dc5dff2542,0}
)";

    // Если передан файл в аргументах, читаем его
    if (argc > 1) {
        std::cout << "Чтение файла: " << argv[1] << std::endl;
        std::ifstream file(argv[1]);
        if (file.is_open()) {
            example_data = std::string(
                (std::istreambuf_iterator<char>(file)),
                std::istreambuf_iterator<char>()
            );
            file.close();
        } else {
            std::cerr << "Ошибка открытия файла!" << std::endl;
            return 1;
        }
    } else {
        std::cout << "Использование встроенных примеров данных" << std::endl;
    }

    std::cout << std::endl;

    // Создаём парсер
    ConfigMetadataParser parser;

    // Парсим данные
    std::cout << "Парсинг метаданных..." << std::endl;
    parser.parseConfigFile(example_data);

    std::cout << std::endl;

    // Выводим сводку
    parser.printSummary();

    std::cout << std::endl;

    // Демонстрация работы с конкретными типами
    std::cout << "=== Примеры работы с API ===" << std::endl;
    std::cout << std::endl;

    // Пример 1: Получение имени типа по GUID
    std::cout << "1. Получение имени типа метаданных:" << std::endl;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;

    std::wstring catalogs_name = parser.getMetadataTypeName(GUID_Catalogs);
    std::cout << "   GUID_Catalogs -> " << converter.to_bytes(catalogs_name) << std::endl;

    std::wstring languages_name = parser.getMetadataTypeName(GUID_Languages);
    std::cout << "   GUID_Languages -> " << converter.to_bytes(languages_name) << std::endl;

    std::cout << std::endl;

    // Пример 2: Генерация имён файлов
    std::cout << "2. Генерация имён файлов:" << std::endl;

    // Для языка
    std::string lang_file = parser.generateFileName(
        GUID_Languages,
        "154c4235-35f5-42c3-a0d5-07b9ea861f14"
    );
    std::cout << "   Язык: " << lang_file << std::endl;

    // Для справочника
    std::string catalog_file = parser.generateFileName(
        GUID_Catalogs,
        "36b34b4a-30fc-40f3-be9d-59e7d39f8e95"
    );
    std::cout << "   Справочник: " << catalog_file << std::endl;

    std::cout << std::endl;

    // Пример 3: Получение детальной информации
    std::cout << "3. Детальная информация о метаданных:" << std::endl;

    auto catalog_info = parser.getMetadataInfo(GUID_Catalogs);
    if (!catalog_info.guid.empty()) {
        std::cout << "   Тип: " << converter.to_bytes(catalog_info.type_name) << std::endl;
        std::cout << "   GUID типа: " << catalog_info.guid << std::endl;
        std::cout << "   Количество объектов: " << catalog_info.count << std::endl;
        std::cout << "   Объекты:" << std::endl;
        for (const auto& item : catalog_info.items) {
            std::cout << "     - " << item << std::endl;
        }
    }

    std::cout << std::endl;

    // Пример 4: Обработка всех метаданных
    std::cout << "4. Генерация имён для всех объектов:" << std::endl;

    const auto& all_metadata = parser.getAllMetadata();
    int total_objects = 0;

    for (const auto& [type_guid, info] : all_metadata) {
        if (info.count > 0) {
            for (const auto& item_guid : info.items) {
                std::string filename = parser.generateFileName(type_guid, item_guid);
                std::cout << "   " << filename << std::endl;
                total_objects++;
            }
        }
    }

    std::cout << std::endl;
    std::cout << "Всего объектов с именами: " << total_objects << std::endl;

    std::cout << std::endl;
    std::cout << "=== Тестирование завершено ===" << std::endl;

    return 0;
}
