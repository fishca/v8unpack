#ifndef COMMANDS_CONCRETE_DEFLATE_COMMAND_H
#define COMMANDS_CONCRETE_DEFLATE_COMMAND_H

#include "../Command.h"

namespace v8unpack {

class DeflateCommand : public Command {
public:
    explicit DeflateCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~DeflateCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_DEFLATE_COMMAND_H
