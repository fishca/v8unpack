#ifndef COMMANDS_CONCRETE_BAT_COMMAND_H
#define COMMANDS_CONCRETE_BAT_COMMAND_H

#include "../Command.h"

namespace v8unpack {

class BatCommand : public Command {
private:
    int generateUnpackParseScript(const std::string& filename);
    int generateBuildScript(const std::string& filename);
    int generateBackupScript(const std::string& filename);

public:
    explicit BatCommand(std::shared_ptr<::MessageRegistrator> logger);
    ~BatCommand() override = default;

    int execute(const std::vector<std::string>& args) override;

    std::string getName() const override;
    std::string getDescription() const override;
    void showUsage() const override;
};

} // namespace v8unpack

#endif // COMMANDS_CONCRETE_BAT_COMMAND_H
