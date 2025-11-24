#include "DeflateCommand.h"
#include "../../V8File.h"
#include <iostream>
#include <algorithm>

namespace v8unpack {

DeflateCommand::DeflateCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int DeflateCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputFile = args[0];
    std::string outputFile = args[1];

    if (logger_) {
        logger_->Status("[DEFLATE] Начало сжатия...");
    }

    std::cout << "Сжатие файла '" << inputFile << "' в '" << outputFile << "'" << std::endl;

    int ret = Deflate(inputFile, outputFile);

    if (ret == 0) {
        std::cout << "Сжатие завершено успешно. Создан файл '" << outputFile << "'" << std::endl;
        if (logger_) {
            logger_->Status("[DEFLATE] Сжатие завершено успешно");
        }
    } else {
        std::cerr << "Ошибка при сжатии файла" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при сжатии файла", "input", inputFile, "output", outputFile);
        }
    }

    return ret;
}

std::string DeflateCommand::getName() const {
    return "deflate";
}

std::string DeflateCommand::getDescription() const {
    return "Сжать файл с использованием алгоритма deflate";
}

void DeflateCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: deflate" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  deflate <input_file> <output_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_file   - путь к входному файлу" << std::endl;
    std::cout << "  output_file - путь к выходному файлу" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  deflate input.txt output.data" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
