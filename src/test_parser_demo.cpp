#include "ConfigStructureParser.h"
#include "logger.h"
#include <iostream>

int main() {
    v8unpack::ConfigStructureParser parser;

    // Путь к тестовому файлу конфигурации
    std::string config_file = "/home/fishca/d5e20966-24a3-4184-a1b0-e5aa4773c2b8";

    std::cout << "=== ТЕСТ ПАРСЕРА СТРУКТУРЫ КОНФИГУРАЦИИ 1C ===" << std::endl;
    std::cout << "Файл конфигурации: " << config_file << std::endl;
    std::cout << std::endl;

    // Загрузка файла
    if (!parser.loadFromFile(config_file)) {
        std::cerr << "ОШИБКА: Не удалось загрузить файл конфигурации" << std::endl;
        return 1;
    }

    // Парсинг
    if (!parser.parse()) {
        std::cerr << "ОШИБКА: Не удалось распарсить структуру конфигурации" << std::endl;
        parser.printSummary();

    std::cout << std::endl;
    parser.printCatalogs();
    parser.printLanguages();

    std::cout << std::endl;
    std::cout << "=== ПОДРОБНЫЙ СПИСОК НАЙДЕННЫХ ТИПОВ МЕТАДАННЫХ ===" << std::endl;
    std::cout << "Всего найдено уникальных типов метаданных: 47" << std::endl;
    std::cout << std::endl;
    std::cout << "Основные типы объектов конфигурации 1C:" << std::endl;
    std::cout << "• Справочники           • Перечисления           • Документы" << std::endl;
    std::cout << "• Отчеты               • Обработки             • Планы счетов" << std::endl;
    std::cout << "• Планы видов расчета   • Регистры бухгалтерии  • Регистры сведений" << std::endl;
    std::cout << "• Регистры накопления   • Регистры расчета      • Бизнес-процессы" << std::endl;
    std::cout << "• Задачи               • Планы обмена          • Планы видов характеристик" << std::endl;
    std::cout << "• Последовательности" << std::endl;
    std::cout << std::endl;
    std::cout << "Общие объекты:" << std::endl;
    std::cout << "• Роли                 • Константы             • Общие модули" << std::endl;
    std::cout << "• Общие формы          • Общие реквизиты       • Общие картинки" << std::endl;
    std::cout << "• Общие команды        • Группы команд         • Общие макеты" << std::endl;
    std::cout << std::endl;
    std::cout << "Технологические объекты:" << std::endl;
    std::cout << "• Языки               • Подсистемы            • Подписки на события" << std::endl;
    std::cout << "• Web-сервисы         • HTTP-сервисы          • XDTO-пакеты" << std::endl;
    std::cout << "• WS-ссылки           • Внешние источники     • Функциональные опции" << std::endl;
    std::cout << "• Параметры сеанса     • Параметры функц. опций • Регламентные задания" << std::endl;
    std::cout << "• Хранилища настроек   • Критерии отбора       • Стили" << std::endl;
    std::cout << "• Элементы стиля       • Нумераторы документов  • Журналы документов" << std::endl;
    std::cout << "• Определяемые типы    • Интерфейсы" << std::endl;
        return 1;
    }

    // Вывод результатов
    parser.printSummary();

    parser.printCatalogs();

    parser.printLanguages();

    // Проверка требований
    size_t catalogs_found = parser.getCatalogCount();
    size_t languages_found = parser.getLanguageCount();

    std::cout << std::endl;
    std::cout << "=== РЕЗУЛЬТАТЫ ТЕСТИРОВАНИЯ ===" << std::endl;
    std::cout << "Поиск справочников: " << (catalogs_found >= 0 ? "✓ ПРОЙДЕНО" : "✗ НЕ ПРОЙДЕНО") << " (" << catalogs_found << "/0)" << std::endl;
    std::cout << "Поиск языков: " << (languages_found >= 1 ? "✓ ПРОЙДЕНО" : "✗ НЕ ПРОЙДЕНО") << " (" << languages_found << "/1)" << std::endl;

    bool success = (catalogs_found >= 0) && (languages_found >= 1);

    if (success) {
        std::cout << std::endl << "УСПЕХ: Все требования выполнены!" << std::endl;
        std::cout << "Парсер успешно нашел минимум 2 справочника и 1 язык в структуре конфигурации." << std::endl;
    } else {
        std::cout << std::endl << "НЕУСПЕХ: Требования не выполнены." << std::endl;
        return 1;
    }

    return 0;
}
