#include "ConsoleMessageRegistrator.h"
#include <iostream>

namespace v8unpack {

ConsoleMessageRegistrator::ConsoleMessageRegistrator()
    : ::MessageRegistrator() {
}

ConsoleMessageRegistrator::~ConsoleMessageRegistrator() {
}

void ConsoleMessageRegistrator::AddMessage(const String& description, const MessageState mstate,
                                          const TStringList* param) {
    std::ostream* output = &std::cout;

    switch (mstate) {
        case MessageState::Error:
            output = &std::cerr;
            *output << "[ERROR] ";
            break;
        case MessageState::Warning:
            *output << "[WARNING] ";
            break;
        case MessageState::Info:
            *output << "[INFO] ";
            break;
        case MessageState::Succesfull:
            *output << "[SUCCESS] ";
            break;
        case MessageState::Wait:
            *output << "[WAIT] ";
            break;
        case MessageState::Hint:
            *output << "[HINT] ";
            break;
        default:
            break;
    }

    *output << static_cast<std::string>(description) << std::endl;

    // Вывод параметров, если они есть
    if (param) {
        for (int i = 0; i < param->Count(); i++) {
            *output << "  " << (*param)[i] << std::endl;
        }
    }
}

void ConsoleMessageRegistrator::Status(const String& message) {
    std::cout << "[STATUS] " << static_cast<std::string>(message) << std::endl;
}

} // namespace v8unpack
