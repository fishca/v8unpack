#include "CommandFactory.h"

// Concrete command includes
#include "concrete/UnpackCommand.h"
#include "concrete/PackCommand.h"
#include "concrete/ParseCommand.h"
#include "concrete/BuildCommand.h"
// #include "concrete/DeflateCommand.h"
// #include "concrete/InflateCommand.h"
#include "concrete/ListCommand.h"
#include "concrete/VersionCommand.h"
#include "concrete/HelpCommand.h"
#include "concrete/ExampleCommand.h"
#include "concrete/BatCommand.h"

#include "../messageregistration.h"

namespace v8unpack {

CommandFactory::CommandFactory(std::shared_ptr<::MessageRegistrator> logger)
    : logger_(logger) {
}

std::shared_ptr<CommandRegistry> CommandFactory::createRegistry() {
    auto registry = std::make_shared<CommandRegistry>(logger_);

    // Register all commands
    registry->registerCommand(createUnpackCommand());
    registry->registerCommand(createPackCommand());
    registry->registerCommand(createParseCommand());
    registry->registerCommand(createBuildCommand());
    // registry->registerCommand(createDeflateCommand());
    // registry->registerCommand(createInflateCommand());
    registry->registerCommand(createListCommand());
    registry->registerCommand(createVersionCommand());
    registry->registerCommand(createHelpCommand());
    registry->registerCommand(createExampleCommand());
    registry->registerCommand(createBatCommand());

    return registry;
}

std::unique_ptr<Command> CommandFactory::createUnpackCommand() {
    return std::make_unique<UnpackCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createPackCommand() {
    return std::make_unique<PackCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createParseCommand() {
    return std::make_unique<ParseCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createBuildCommand() {
    return std::make_unique<BuildCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createListCommand() {
    return std::make_unique<ListCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createVersionCommand() {
    return std::make_unique<VersionCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createHelpCommand() {
    auto helpCommand = std::make_unique<HelpCommand>(logger_);
    // Registry will be set after it's created to avoid recursion
    return helpCommand;
}

std::unique_ptr<Command> CommandFactory::createExampleCommand() {
    return std::make_unique<ExampleCommand>(logger_);
}

std::unique_ptr<Command> CommandFactory::createBatCommand() {
    return std::make_unique<BatCommand>(logger_);
}

} // namespace v8unpack
