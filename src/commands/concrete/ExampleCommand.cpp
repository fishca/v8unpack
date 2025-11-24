#include "ExampleCommand.h"
#include <iostream>

namespace v8unpack {

ExampleCommand::ExampleCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int ExampleCommand::execute(const std::vector<std::string>& args) {
    if (!args.empty()) {
        showUsage();
        return 1;
    }

    std::cout << "Примеры использования v8unpack:" << std::endl << std::endl;

    std::cout << "=== Разпаковка конфигурации ===" << std::endl;
    std::cout << "v8unpack -unpack config.cf unpacked/" << std::endl;
    std::cout << "# Разпакует 'config.cf' в папку 'unpacked/'" << std::endl << std::endl;

    std::cout << "=== Упаковка конфигурации ===" << std::endl;
    std::cout << "v8unpack --pack unpacked/ config_new.cf" << std::endl;
    std::cout << "# Упакует папку 'unpacked/' в 'config_new.cf'" << std::endl << std::endl;

    std::cout << "=== Просмотр содержимого ===" << std::endl;
    std::cout << "v8unpack list config.cf" << std::endl;
    std::cout << "# Показывает список файлов внутри контейнера" << std::endl << std::endl;

    std::cout << "=== Парсинг в текстовую форму ===" << std::endl;
    std::cout << "v8unpack -parse config.cf parsed/" << std::endl;
    std::cout << "# Парсит метаданные и сохранит в файлы" << std::endl << std::endl;

    std::cout << "=== Сборка из файловой структуры ===" << std::endl;
    std::cout << "v8unpack -build unpacked/ new_config.cf" << std::endl;
    std::cout << "# Собрать и упаковать из файловой структуры" << std::endl << std::endl;

    std::cout << "=== Сжатие файла deflate ===" << std::endl;
    std::cout << "v8unpack -deflate input.txt output.deflated" << std::endl;
    std::cout << "# Сжать файл с помощью deflate" << std::endl << std::endl;

    std::cout << "=== Распаковка deflate ===" << std::endl;
    std::cout << "v8unpack -inflate input.deflated output.txt" << std::endl;
    std::cout << "# Распаковать deflate файл" << std::endl << std::endl;

    std::cout << "=== Просмотр версии ===" << std::endl;
    std::cout << "v8unpack --version" << std::endl;
    std::cout << "# Показывает версию программы" << std::endl << std::endl;

    std::cout << "=== Получение справки ===" << std::endl;
    std::cout << "v8unpack help" << std::endl;
    std::cout << "# Показать общую справку по командам" << std::endl;
    std::cout << "v8unpack help unpack" << std::endl;
    std::cout << "# Показать справку по команде unpack" << std::endl << std::endl;

    std::cout << "=== Работа с отчётами (.erf) ===" << std::endl;
    std::cout << "v8unpack -unpack report.erf report_data/" << std::endl;
    std::cout << "v8unpack -parse report.erf parsed_report/" << std::endl << std::endl;

    std::cout << "=== Работа с обработками (.epf) ===" << std::endl;
    std::cout << "v8unpack -unpack processing.epf processing_code/" << std::endl;
    std::cout << "v8unpack -parse processing.epf parsed_processing/" << std::endl << std::endl;

    return 0;
}

std::string ExampleCommand::getName() const {
    return "example";
}

std::string ExampleCommand::getDescription() const {
    return "Показать примеры использования v8unpack";
}

void ExampleCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: example" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  example" << std::endl;
    std::cout << std::endl;
    std::cout << "Пример:" << std::endl;
    std::cout << "  example" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
