#include "BatCommand.h"
#include <iostream>
#include <fstream>
#include <string>

namespace v8unpack {

BatCommand::BatCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int BatCommand::execute(const std::vector<std::string>& args) {
    if (args.size() < 2) {
        showUsage();
        return 1;
    }

    std::string scriptType = args[0];
    std::string outputFile = args[1];

    if (scriptType == "unpack-parse") {
        return generateUnpackParseScript(outputFile);
    } else if (scriptType == "build") {
        return generateBuildScript(outputFile);
    } else if (scriptType == "backup") {
        return generateBackupScript(outputFile);
    } else {
        std::cerr << "Неизвестный тип скрипта: " << scriptType << std::endl;
        showUsage();
        return 1;
    }
}

int BatCommand::generateUnpackParseScript(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка создания файла: " << filename << std::endl;
        return 1;
    }

    file << "@echo off" << std::endl;
    file << "echo Unpack and Parse Script" << std::endl;
    file << "echo =====================" << std::endl;
    file << std::endl;
    file << "set V8UNPACK_PATH=v8unpack.exe" << std::endl;
    file << "set INPUT_FILE=%1" << std::endl;
    file << "set OUTPUT_DIR=%2" << std::endl;
    file << std::endl;
    file << "if \"%INPUT_FILE%\"==\"\" (" << std::endl;
    file << "    echo Usage: %0 input_file.cf output_directory" << std::endl;
    file << "    exit /b 1" << std::endl;
    file << ")" << std::endl;
    file << std::endl;
    file << "if not exist \"%V8UNPACK_PATH%\" (" << std::endl;
    file << "    echo Error: v8unpack.exe not found at %V8UNPACK_PATH%" << std::endl;
    file << "    exit /b 1" << std::endl;
    file << ")" << std::endl;
    file << std::endl;
    file << "echo Unpacking %INPUT_FILE% to %OUTPUT_DIR%..." << std::endl;
    file << "%V8UNPACK_PATH% --unpack \"%INPUT_FILE%\" \"%OUTPUT_DIR%\"" << std::endl;
    file << "if errorlevel 1 exit /b 1" << std::endl;
    file << std::endl;
    file << "echo Parsing metadata..." << std::endl;
    file << "%V8UNPACK_PATH% --parse \"%INPUT_FILE%\" \"%OUTPUT_DIR%\\parsed\"" << std::endl;
    file << std::endl;
    file << "echo Script completed successfully." << std::endl;

    file.close();

    std::cout << "Создан скрипт unpack-parse: " << filename << std::endl;
    return 0;
}

int BatCommand::generateBuildScript(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка создания файла: " << filename << std::endl;
        return 1;
    }

    file << "@echo off" << std::endl;
    file << "echo Build Configuration Script" << std::endl;
    file << "echo =========================" << std::endl;
    file << std::endl;
    file << "set V8UNPACK_PATH=v8unpack.exe" << std::endl;
    file << "set INPUT_DIR=%1" << std::endl;
    file << "set OUTPUT_FILE=%2" << std::endl;
    file << std::endl;
    file << "if \"%INPUT_DIR%\"==\"\" (" << std::endl;
    file << "    echo Usage: %0 input_directory output_file.cf" << std::endl;
    file << "    exit /b 1" << std::endl;
    file << ")" << std::endl;
    file << std::endl;
    file << "if not exist \"%V8UNPACK_PATH%\" (" << std::endl;
    file << "    echo Error: v8unpack.exe not found at %V8UNPACK_PATH%" << std::endl;
    file << "    exit /b 1" << std::endl;
    file << ")" << std::endl;
    file << std::endl;
    file << "echo Building configuration from %INPUT_DIR% to %OUTPUT_FILE%..." << std::endl;
    file << "%V8UNPACK_PATH% --build \"%INPUT_DIR%\" \"%OUTPUT_FILE%\"" << std::endl;
    file << "if errorlevel 1 exit /b 1" << std::endl;
    file << std::endl;
    file << "echo Build completed successfully." << std::endl;

    file.close();

    std::cout << "Создан скрипт build: " << filename << std::endl;
    return 0;
}

int BatCommand::generateBackupScript(const std::string& filename) {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Ошибка создания файла: " << filename << std::endl;
        return 1;
    }

    file << "@echo off" << std::endl;
    file << "echo Backup Configuration Script" << std::endl;
    file << "echo ============================" << std::endl;
    file << std::endl;
    file << "set V8UNPACK_PATH=v8unpack.exe" << std::endl;
    file << "set CONFIG_FILE=%1" << std::endl;
    file << "set BACKUP_DIR=%2" << std::endl;
    file << std::endl;
    file << "if \"%CONFIG_FILE%\"==\"\" (" << std::endl;
    file << "    echo Usage: %0 config.cf backup_directory" << std::endl;
    file << "    exit /b 1" << std::endl;
    file << ")" << std::endl;
    file << std::endl;
    file << "if not exist \"%V8UNPACK_PATH%\" (" << std::endl;
    file << "    echo Error: v8unpack.exe not found at %V8UNPACK_PATH%" << std::endl;
    file << "    exit /b 1" << std::endl;
    file << ")" << std::endl;
    file << std::endl;
    file << "set TIMESTAMP=%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%" << std::endl;
    file << "set TIMESTAMP=%TIMESTAMP: =0%" << std::endl;
    file << std::endl;
    file << "echo Creating backup at %BACKUP_DIR%\\backup_%TIMESTAMP%..." << std::endl;
    file << "%V8UNPACK_PATH% --unpack \"%CONFIG_FILE%\" \"%BACKUP_DIR%\\backup_%TIMESTAMP%\"" << std::endl;
    file << "if errorlevel 1 exit /b 1" << std::endl;
    file << std::endl;
    file << "%V8UNPACK_PATH% --parse \"%CONFIG_FILE%\" \"%BACKUP_DIR%\\backup_%TIMESTAMP%\\parsed\"" << std::endl;
    file << std::endl;
    file << "echo Backup completed at %BACKUP_DIR%\\backup_%TIMESTAMP%" << std::endl;

    file.close();

    std::cout << "Создан скрипт backup: " << filename << std::endl;
    return 0;
}

std::string BatCommand::getName() const {
    return "bat";
}

std::string BatCommand::getDescription() const {
    return "Создать bat скрипт для автоматизации задач v8unpack";
}

void BatCommand::showUsage() const {
    std::cout << std::endl;
    std::cout << "Команда: bat" << std::endl;
    std::cout << getDescription() << std::endl;
    std::cout << std::endl;
    std::cout << "Использование:" << std::endl;
    std::cout << "  bat <script_type> <output_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Типы скриптов:" << std::endl;
    std::cout << "  unpack-parse   - скрипт для распаковки и парсинга" << std::endl;
    std::cout << "  build          - скрипт для сборки конфигурации" << std::endl;
    std::cout << "  backup         - скрипт для создания резервной копии с распаковкой" << std::endl;
    std::cout << std::endl;
    std::cout << "Примеры:" << std::endl;
    std::cout << "  bat unpack-parse unpack_script.bat" << std::endl;
    std::cout << "  bat build build_config.bat" << std::endl;
    std::cout << "  bat backup backup_config.bat" << std::endl;
    std::cout << std::endl;
}

} // namespace v8unpack
