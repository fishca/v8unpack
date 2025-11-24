#ifndef COMMANDS_CONCRETE_BUILD_COMMAND_H
#define COMMANDS_CONCRETE_BUILD_COMMAND_H

#include "../Command.h"

namespace v8unpack {

class BuildCommand : public Command {
private:
    bool pack_ = true; // default to pack, modify with -nopack

public:
    explicit BuildCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~BuildCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;

    // Support modifiers
    bool supportsModifier(const std::string& modifier) const override;
    void setModifier(const std::string& modifier, bool value) override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_BUILD_COMMAND_H
