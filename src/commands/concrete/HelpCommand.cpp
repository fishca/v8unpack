#include "HelpCommand.h"
#include <iostream>

namespace v8unpack {

HelpCommand::HelpCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

void HelpCommand::setRegistry(std::shared_ptr<CommandRegistry> registry) {
    registry_ = registry;
}

int HelpCommand::execute(const std::vector<std::string>& args) {
    if (args.empty()) {
        // Показать общую справку по всем командам
        if (registry_) {
            registry_->showHelp();
        } else {
            showGeneralHelp();
        }
        return 0;
    } else if (args.size() == 1) {
        // Показать справку по конкретной команде
        std::string commandName = args[0];
        if (registry_) {
            registry_->showCommandHelp(commandName);
        }
        return 0;
    } else {
        // Слишком много аргументов
        showUsage();
        return 1;
    }
}

std::string HelpCommand::getName() const {
    return "help";
}

std::string HelpCommand::getDescription() const {
    return "Показать справку по командам";
}

void HelpCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: help" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  help              - показать общую справку по всем командам" << std::endl;
    std::cout << "  help <command>    - показать справку по конкретной команде" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  help              - список всех команд" << std::endl;
    std::cout << "  help build        - справка по команде build" << std::endl;
    std::cout << std::endl;
}

void HelpCommand::showGeneralHelp() {
    std::cout << std::endl;
    std::cout << "V8Unpack - утилита для работы с файлами конфигураций 1C:Предприятие v8" << std::endl;
    std::cout << std::endl;
    std::cout << "Доступные команды:" << std::endl;
    std::cout << std::endl;
    std::cout << " Основные команды:" << std::endl;
    std::cout << "  unpack <file> <dir>      - распаковать контейнер" << std::endl;
    std::cout << "  pack <dir> <file>        - упаковать контейнер" << std::endl;
    std::cout << "  parse <file> <dir>       - разобрать метаданные" << std::endl;
    std::cout << "  build <dir> <file>       - собрать из структуры" << std::endl;
    std::cout << "  version                  - показать версию" << std::endl;
    std::cout << std::endl;
    std::cout << " Команды сжатия:" << std::endl;
    std::cout << "  deflate <in> <out>       - сжать файл" << std::endl;
    std::cout << "  inflate <in> <out>       - распаковать файл" << std::endl;
    std::cout << std::endl;
    std::cout << " Вспомогательные команды:" << std::endl;
    std::cout << "  list <file>              - показать содержимое контейнера" << std::endl;
    std::cout << "  example                  - показать примеры использования" << std::endl;
    std::cout << "  bat <type> <file>        - создать bat скрипт" << std::endl;
    std::cout << "  help [command]           - показать эту справку" << std::endl;
    std::cout << std::endl;
    std::cout << " Используйте 'help <command>' для подробной справки по команде." << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
