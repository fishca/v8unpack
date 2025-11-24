#ifndef COMMANDS_CONCRETE_PARSE_COMMAND_H
#define COMMANDS_CONCRETE_PARSE_COMMAND_H

#include "../Command.h"

namespace v8unpack {

class ParseCommand : public Command {
public:
    explicit ParseCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~ParseCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_PARSE_COMMAND_H
