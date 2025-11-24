#ifndef COMMANDS_CONCRETE_EXAMPLE_COMMAND_H
#define COMMANDS_CONCRETE_EXAMPLE_COMMAND_H

#include "../Command.h"

namespace v8unpack {

class ExampleCommand : public Command {
public:
    explicit ExampleCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~ExampleCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_EXAMPLE_COMMAND_H
