#include "InflateCommand.h"
#include "../../V8File.h"
#include <iostream>
#include <algorithm>

namespace v8unpack {

InflateCommand::InflateCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int InflateCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputFile = args[0];
    std::string outputFile = args[1];

    if (logger_) {
        logger_->Status("[INFLATE] Начало распаковки...");
    }

    std::cout << "Распаковка файла '" << inputFile << "' в '" << outputFile << "'" << std::endl;

    int ret = Inflate(inputFile, outputFile);

    if (ret == 0) {
        std::cout << "Распаковка завершена успешно. Создан файл '" << outputFile << "'" << std::endl;
        if (logger_) {
            logger_->Status("[INFLATE] Распаковка завершена успешно");
        }
    } else {
        std::cerr << "Ошибка при распаковке файла" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при распаковке файла", "input", inputFile, "output", outputFile);
        }
    }

    return ret;
}

std::string InflateCommand::getName() const {
    return "inflate";
}

std::string InflateCommand::getDescription() const {
    return "Распаковать файл, сжатый алгоритмом deflate";
}

void InflateCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: inflate" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  inflate <input_file> <output_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_file   - путь к входному сжатому файлу" << std::endl;
    std::cout << "  output_file - путь к выходному распакованному файлу" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  inflate compressed.data decompressed.txt" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
