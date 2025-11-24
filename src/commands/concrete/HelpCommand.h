#ifndef COMMANDS_CONCRETE_HELP_COMMAND_H
#define COMMANDS_CONCRETE_HELP_COMMAND_H

#include "../Command.h"
#include "../CommandRegistry.h"

namespace v8unpack {

class HelpCommand : public Command {
private:
    std::shared_ptr<CommandRegistry> registry_;

    void showGeneralHelp();

public:
    explicit HelpCommand(std::shared_ptr<::MessageRegistrator> logger = nullptr);
    void setRegistry(std::shared_ptr<CommandRegistry> registry);

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_HELP_COMMAND_H
