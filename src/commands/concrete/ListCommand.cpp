#include "ListCommand.h"
#include "../../V8File.h"
#include <iostream>

namespace v8unpack {

ListCommand::ListCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int ListCommand::execute(const std::vector<std::string>& args) {
    if (args.size() != 1) {
        showUsage();
        return 1;
    }

    std::string inputFile = args[0];

    if (logger_) {
        logger_->Status("[LIST] Начало просмотра содержимого...");
    }

    std::cout << "Содержимое контейнера '" << inputFile << "':" << std::endl;

    int ret = ListFiles(inputFile);

    if (ret == 0) {
        if (logger_) {
            logger_->Status("[LIST] Просмотр завершён");
        }
    } else {
        std::cerr << "Ошибка при просмотре содержимого файла" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при просмотре содержимого", "file", inputFile);
        }
    }

    return ret;
}

std::string ListCommand::getName() const {
    return "list";
}

std::string ListCommand::getDescription() const {
    return "Показать список файлов в контейнере 1C v8";
}

void ListCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: list" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  list <input_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_file   - путь к файлу (.cf, .epf, .erf)" << std::endl;
    std::cout << std::endl;
    std::cout << "Пример:" << std::endl;
    std::cout << "  list config.cf" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
