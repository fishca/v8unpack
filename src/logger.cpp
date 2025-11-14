#include "logger.h"

void Logger::log(const std::string& message)
{
	std::time_t now = std::time(nullptr);
	std::string timestamp = std::ctime(&now);
	timestamp.pop_back();
	logfile << timestamp << " - " << message << std::endl;

}
