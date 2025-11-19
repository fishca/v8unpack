#include "V8MetaParser.h"
#include "V8File.h"  // For logger and related functions
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <fstream>

namespace v8unpack {

std::vector<std::string> find_guids(const std::string& text) {
    std::vector<std::string> guids;
    std::regex guid_pattern(R"(\{([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})\})");
    std::sregex_iterator iter(text.begin(), text.end(), guid_pattern);
    std::sregex_iterator end;

    for (; iter != end; ++iter) {
        std::string guid = iter->str(1);
        std::transform(guid.begin(), guid.end(), guid.begin(), ::tolower);
        guids.push_back(guid);
    }

    return guids;
}

std::string extract_object_name(const std::string& content, const std::string& expected_guid) {
    // Simple implementation - in real use would parse 1C format
    return "Unknown_Object";
}

bool ensure_directory(const fs::path& path) {
    boost::system::error_code ec;
    if (!fs::exists(path)) {
        if (!fs::create_directories(path, ec)) {
            return false;
        }
    }
    return true;
}

// Simple stub implementation for now
int ParseToStringWithFiles(const std::string &config_string, const std::string &dirname) {
    // Create base directory
    fs::path base_dir(dirname);
    if (!ensure_directory(base_dir)) {
        std::cerr << "Failed to create base directory: " << dirname << std::endl;
        return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
    }

    // For now, just save the entire config string to a single file
    fs::path out_file = base_dir / "config.txt";
    std::ofstream file(out_file.string(), std::ios::binary);
    if (!file) {
        return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
    }

    file.write(config_string.c_str(), config_string.size());
    file.close();

    return V8UNPACK_OK;
}

} // namespace v8unpack
