#include "BuildCommand.h"
#include "../../V8File.h"
#include <iostream>
#include <algorithm>

namespace v8unpack {

BuildCommand::BuildCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger), pack_(true) {
}

int BuildCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string inputDir = args[0];
    std::string outputFile = args[1];

    if (logger_) {
        logger_->Status("[BUILD] Начало сборки контейнера...");
    }

    std::string operation = pack_ ? "Сборка и упаковка" : "Сборка без упаковки";
    std::cout << operation << " директории '" << inputDir << "' в файл '" << outputFile << "'" << std::endl;

    int ret = BuildCfFile(inputDir, outputFile, pack_);

    if (ret == 0) {
        std::string successMsg = pack_ ?
            "Сборка и упаковка завершены успешно. Создан файл '" + outputFile + "'" :
            "Сборка завершена успешно. Создан файл '" + outputFile + "' (без упаковки)";
        std::cout << successMsg << std::endl;
        if (logger_) {
            logger_->Status("[BUILD] Сборка завершена успешно");
        }
    } else {
        std::cerr << "Ошибка при сборке контейнера" << std::endl;
        if (logger_) {
            logger_->AddError("Ошибка при сборке контейнера", "input", inputDir, "output", outputFile);
        }
    }

    return ret;
}

std::string BuildCommand::getName() const {
    return "build";
}

std::string BuildCommand::getDescription() const {
    return "Собрать контейнер из файловой структуры";
}

void BuildCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: build" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  build <input_dir> <output_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Параметры:" << std::endl;
    std::cout << "  input_dir   - путь к входной директории с распакованными файлами" << std::endl;
    std::cout << "  output_file - путь к выходному файлу (.cf, .epf, .erf)" << std::endl;
    std::cout << std::endl;
    std::cout << "Модификаторы:" << std::endl;
    std::cout << "  -nopack     - собрать без упаковки (временные файлы)" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  build unpacked/ config_new.cf" << std::endl;
    std::cout << "  build unpacked/ temp_files/ -nopack" << std::endl;
    std::cout << std::endl;
}

bool BuildCommand::supportsModifier(const std::string& modifier) const {
    return modifier == "nopack";
}

void BuildCommand::setModifier(const std::string& modifier, bool value) {
    if (modifier == "nopack") {
        pack_ = !value; // when -nopack is set, pack becomes false
    }
}

} // namespace v8unpack
