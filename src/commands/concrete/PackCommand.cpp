#include "PackCommand.h"
#include "../../V8File.h"
#include <iostream>
#include <algorithm>

namespace v8unpack {

PackCommand::PackCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int PackCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputDir = args[0];
    std::string outputFile = args[1];

    if (logger_) {
        logger_->Status("[PACK] Начало упаковки...");
    }

    std::cout << "Упаковка каталога '" << inputDir << "' в файл '" << outputFile << "'" << std::endl;

    int ret = PackFromFolder(inputDir, outputFile);

    if (ret == 0) {
        std::cout << "Упаковка завершена успешно. Создан файл '" << outputFile << "'" << std::endl;
        if (logger_) {
            logger_->Status("[PACK] Упаковка завершена успешно");
        }
    } else {
        std::cerr << "Ошибка при упаковке каталога" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при упаковке каталога", "input", inputDir, "output", outputFile);
        }
    }

    return ret;
}

std::string PackCommand::getName() const {
    return "pack";
}

std::string PackCommand::getDescription() const {
    return "Упаковать файловую структуру обратно в контейнер 1C v8 (.cf, .epf, .erf)";
}

void PackCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: pack" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  pack <input_dir> <output_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_dir   - путь к входному каталогу с распакованными файлами" << std::endl;
    std::cout << "  output_file - путь к выходному файлу (.cf, .epf, .erf)" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  pack unpacked/ config_new.cf" << std::endl;
    std::cout << "  pack report_data/ report_new.erf" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
