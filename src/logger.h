#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <ctime>

class Logger {
public:
	Logger(const std::string& filename) : logfile(filename, std::ios::app) {}
	void log(const std::string& message);
private:
	std::ofstream logfile;
};

#endif // LOGGER_H
