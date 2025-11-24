#include "ParseCommand.h"
#include "../../V8File.h"
#include <iostream>
#include <algorithm>

namespace v8unpack {

ParseCommand::ParseCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int ParseCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputFile = args[0];
    std::string outputDir = args[1];
    std::vector<std::string> filter;

    // Parse additional filter arguments (block names)
    for (size_t i = 2; i < args.size(); ++i) {
        if (!args[i].empty()) {
            filter.push_back(args[i]);
        }
    }

    if (logger_) {
        logger_->Status("[PARSE] Начало парсинга...");
    }

    std::cout << "Парсинг файла '" << inputFile << "' в каталог '" << outputDir << "'" << std::endl;

    int ret = Parse(inputFile, outputDir, filter);

    if (ret == 0) {
        std::cout << "Парсинг завершён успешно. Создан каталог '" << outputDir << "'" << std::endl;
        if (logger_) {
            logger_->Status("[PARSE] Парсинг завершён успешно");
        }
    } else {
        std::cerr << "Ошибка при парсинге файла" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при парсинге файла", "input", inputFile, "output", outputDir);
        }
    }

    return ret;
}

std::string ParseCommand::getName() const {
    return "parse";
}

std::string ParseCommand::getDescription() const {
    return "Проанализировать и сохранить файлы метаданных контейнера 1C v8";
}

void ParseCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: parse" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  parse <input_file> <output_dir> [block_name1] [block_name2] ..." << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_file   - путь к входному файлу (.cf, .epf, .erf)" << std::endl;
    std::cout << "  output_dir   - путь к выходному каталогу" << std::endl;
    std::cout << "  block_name   - опционально, имена блоков для парсинга (фильтр)" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  parse config.cf parsed/" << std::endl;
    std::cout << "  parse report.erf report_data/ root metadata" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
