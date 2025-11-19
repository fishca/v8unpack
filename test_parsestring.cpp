#include "V8File.h"
#include <iostream>
#include <vector>

using namespace v8unpack;

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <cf_file>" << std::endl;
        return 1;
    }

    std::string config_string;
    std::vector<std::string> filter;

    int ret = ParseToString(argv[1], filter, config_string);

    if (ret == 0) {
        std::cout << "ParseToString success. Result:" << std::endl;
        std::cout << config_string << std::endl;
    } else {
        std::cout << "ParseToString failed with code: " << ret << std::endl;
    }

    return ret;
}
