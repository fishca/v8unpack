#ifndef COMMANDS_CONCRETE_INFLATE_COMMAND_H
#define COMMANDS_CONCRETE_INFLATE_COMMAND_H

#include "../Command.h"

namespace v8unpack {

class InflateCommand : public Command {
public:
    explicit InflateCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~InflateCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_INFLATE_COMMAND_H
