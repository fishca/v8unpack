#include "UnpackCommand.h"
#include "../../V8File.h"
#include <iostream>
#include <algorithm>

namespace v8unpack {

UnpackCommand::UnpackCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int UnpackCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputFile = args[0];
    std::string outputDir = args[1];
    std::string blockName;

    if (args.size() >= 3) {
        blockName = args[2];
    }

    if (logger_) {
        logger_->Status("[UNPACK] Начало распаковки...");
    }

    std::cout << "Распаковка файла '" << inputFile << "' в каталог '" << outputDir << "'" << std::endl;

    int ret = UnpackToFolder(inputFile, outputDir, blockName, true);

    if (ret == 0) {
        std::cout << "Распаковка завершена успешно в каталог '" << outputDir << "'" << std::endl;
        if (logger_) {
            logger_->Status("[UNPACK] Распаковка завершена успешно");
        }
    } else {
        std::cerr << "Ошибка при распаковке файла" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при распаковке файла", "input", inputFile, "output", outputDir);
        }
    }

    return ret;
}

std::string UnpackCommand::getName() const {
    return "unpack";
}

std::string UnpackCommand::getDescription() const {
    return "Распаковать контейнер 1C v8 (.cf, .epf, .erf) в файловую структуру";
}

void UnpackCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: unpack" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  unpack <input_file> <output_dir> [block_name]" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_file   - путь к входному файлу (.cf, .epf, .erf)" << std::endl;
    std::cout << "  output_dir   - путь к выходному каталогу" << std::endl;
    std::cout << "  block_name   - опционально, имя блока для распаковки" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  unpack config.cf unpacked/" << std::endl;
    std::cout << "  unpack report.erf report_data/ root" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
