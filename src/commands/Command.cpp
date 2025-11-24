#include "Command.h"
#include "messageregistration.h"

namespace v8unpack {

Command::Command(std::shared_ptr<::MessageRegistrator> logger)
    : logger_(logger) {
}

bool Command::supportsModifier(const std::string& modifier) const {
    // По умолчанию модификаторы не поддерживаются
    return false;
}

void Command::setModifier(const std::string& modifier, bool value) {
    // По умолчанию ничего не делаем
    // Конкретные команды могут переопределить это поведение
}

} // namespace v8unpack
