#include "VersionCommand.h"
#include "../../version.h"
#include <iostream>

namespace v8unpack {

VersionCommand::VersionCommand(std::shared_ptr<::MessageRegistrator> logger)
    : Command(logger) {
}

int VersionCommand::execute(const std::vector<std::string>& args) {
    std::cout << V8P_VERSION;

    // Определяем архитектуру на основе размера указателя
#if defined(__x86_64__) || defined(_M_X64)
    std::cout << " (x64)" << std::endl;
#elif defined(__i386__) || defined(_M_IX86) || defined(__WORDSIZE) && __WORDSIZE == 32
    std::cout << " (x32)" << std::endl;
#elif defined(__WORDSIZE) && __WORDSIZE == 64
    std::cout << " (x64)" << std::endl;
#else
    // Альтернативный способ определения по размеру void*
    // Для демонстрации заменим условие: показать что было бы для 32-бит
    if (sizeof(void*) == 8) {
        std::cout << " (x64)" << std::endl;
    } else if (sizeof(void*) == 4) {
        std::cout << " (x32)" << std::endl;  // Это условие выполнится в 32-битной системе
    } else {
        std::cout << " (unk" << (sizeof(void*) * 8) << "bit) " << std::endl;
    }
#endif

    return 0;
}

std::string VersionCommand::getName() const {
    return "version";
}

std::string VersionCommand::getDescription() const {
    return "Показать версию программы";
}

void VersionCommand::showUsage() const {
    std::cout << "Использование: v8unpack version" << std::endl;
    std::cout << "Показывает номер версии программы и архитектуру системы." << std::endl;
}

} // namespace v8unpack
