#include "V8MetaParser.h"
#include "V8File.h"  // For logger and related functions
#include "logger.h"  // For Logger extern declaration
#include <regex>
#include <algorithm>
#include <unordered_map>
#include <iostream>
#include <fstream>

namespace v8unpack {

extern Logger logger;

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

int ParseToStringWithFiles(const std::string &config_string, const std::string &dirname) {
    // Create base directory
    fs::path base_dir(dirname);
    if (!ensure_directory(base_dir)) {
        std::cerr << "ParseToStringWithFiles: Failed to create base directory: " << dirname << std::endl;
        return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
    }

    // Create "Конфигурация" directory for configuration files
    fs::path config_dir = base_dir / "Конфигурация";
    if (!ensure_directory(config_dir)) {
        std::cerr << "ParseToStringWithFiles: Failed to create configuration directory: " << config_dir.string() << std::endl;
        return V8UNPACK_ERROR_CREATING_OUTPUT_FILE;
    }

    // Metadata type mapping based on root GUID
    std::unordered_map<std::string, std::string> guid_to_type = {
        // Subsystems
        {"37f2fa9a-b276-11d4-9435-004095e12fc7", "Подсистемы"},
        // Common modules, attributes, forms, templates, commands, pictures, XDTO packages, web services, HTTP services, WS references
        {"0fe48980-252d-11d6-a3c7-0050bae0a776", "Общие_модули"},
        {"15794563-ccec-41f6-a83c-ec5f7b9a5bc1", "Общие_реквизиты"},
        {"07ee8426-87f1-11d5-b99c-0050bae0a95d", "Общие_формы"},
        {"0c89c792-16c3-11d5-b96b-0050bae0a95d", "Общие_макеты"},
        {"2f1a5187-fb0e-4b05-9489-dc5dd6412348", "Общие_команды"},
        {"7dcd43d9-aca5-4926-b549-1842e6a4e8cf", "Общие_картинки"},
        {"cc9df798-7c94-4616-97d2-7aa0b7bc515e", "XDTO_пакеты"},
        {"8657032e-7740-4e1d-a3ba-5dd6e8afb78f", "Web_сервисы"},
        {"0fffc09c-8f4c-47cc-b41c-8d5c5a221d79", "HTTP_сервисы"},
        {"d26096fb-7a5d-4df9-af63-47d04771fa9b", "WS_ссылки"},
        // Styles, languages, interfaces
        {"3e5404af-6ef8-4c73-ad11-91bd2dfac4c8", "Стили"},
        {"9cd510cd-abfc-11d4-9434-004095e12fc7", "Языки"},
        {"39bddf6a-0c3c-452b-921c-d99cfa1c2f1b", "Интерфейсы"},
        // Roles, session parameters, exchange plans, filter criteria, event subscriptions, scheduled jobs, functional options
        {"09736b02-9cac-4e3f-b4f7-d3e9576ab948", "Роли"},
        {"24c43748-c938-45d0-8d14-01424a72b11e", "Параметры_сеанса"},
        {"857c4a91-e5f4-4fac-86ec-787626f1c108", "Планы_обмена"},
        {"3e7bfcc0-067d-11d6-a3c7-0050bae0a776", "Критерии_отбора"},
        {"4e828da6-0f44-4b5b-b1c0-a2b3cfe7bdcc", "Подписки_на_события"},
        {"11bdaf85-d5ad-4d91-bb24-aa0eee139052", "Регламентные_задания"},
        {"af547940-3268-434f-a3e7-e47d6d2638c3", "Функциональные_опции"},
        // Defined types, settings storages, command groups, style items
        {"c045099e-13b9-4fb6-9d50-fca00202971e", "Определяемые_типы"},
        {"46b4cd97-fd13-4eaa-aba2-3bddd7699218", "Хранилища_настроек"},
        {"1c57eabe-7349-44b3-b1de-ebfeab67b47d", "Группы_команд"},
        {"58848766-36ea-4076-8800-e91eb49590d7", "Элементы_стиля"},
        // Constants, documents, documents journals, enumerations
        {"0195e80c-b157-11d4-9435-004095e12fc7", "Константы"},
        {"061d872a-5787-460e-95ac-ed74ea3a3e84", "Документы"},
        {"4612bd75-71b7-4a5c-8cc5-2b0b65f9fa0d", "Журналы_документов"},
        {"f6a80749-5ad7-400b-8519-39dc5dff2542", "Перечисления"},
        // Catalogs, reports, data processors
        {"cf4abea6-37b2-11d4-940f-008048da11f9", "Справочники"},
        {"631b75a0-29e2-11d6-a3c7-0050bae0a776", "Отчеты"},
        {"bf845118-327b-4682-b5c6-285d2a0eb296", "Обработки"},
        // Charts of characteristic types, accounts, calculation types
        {"82a1b659-b220-4d94-a9bd-14d757b95a48", "Планы_видов_характеристик"},
        {"238e7e88-3c5f-48b2-8a3b-81ebbecb20ed", "Планы_счетов"},
        {"30b100d6-b29f-47ac-aec7-cb8ca8a54767", "Планы_видов_расчета"},
        // Information registers, accumulation registers, accounting registers, calculation registers
        {"13134201-f60b-11d5-a3c7-0050bae0a776", "Регистры_сведений"},
        {"b64d9a40-1642-11d6-a3c7-0050bae0a776", "Регистры_накопления"},
        {"2deed9b8-0056-4ffe-a473-c20a6c32a0bc", "Регистры_бухгалтерии"},
        {"f2de87a8-64e5-45eb-a22d-b3aedab050e7", "Регистры_расчета"},
        // Business processes, tasks, external data sources
        {"fcd3404e-1523-48ce-9bc0-ecdb822684a1", "Бизнес_процессы"},
        {"3e63355c-1378-4953-be9b-1deb5fb6bec5", "Задачи"},
        {"5274d9fc-9c3a-4a71-8f5e-a0db8ab23de5", "Внешние_источники_данных"}
    };

    // Map to store which objects belong to which metadata types
    std::unordered_map<std::string, std::vector<std::string>> type_to_objects;

    // Split string into sections based on "--- " delimiter and analyze configuration structure
    std::istringstream stream(config_string);
    std::string line;
    std::string current_section;
    std::string current_data;
    int file_count = 0;
    const std::string config_guid_pattern = R"(\{([^,]+)\})"; // Find first GUID (configuration GUID)
    const std::string metadata_section_pattern = R"(\{([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12}),(\d+),([^{}]*)\})";

    std::regex config_guid_regex(config_guid_pattern);
    std::regex metadata_section_regex(metadata_section_pattern);
    std::string config_guid;

    while (std::getline(stream, line)) {
        if (line.find("--- ") == 0 && line.find(" ---") != std::string::npos) {
            // Process previous section if exists
            if (!current_section.empty() && !current_data.empty()) {
                // Check if this is the configuration file
                if (current_section.find("root") != std::string::npos ||
                    current_section == "version" ||
                    current_section == "versions" ||
                    current_section == "metadata") {

                    // Save configuration files directly
                    std::string filename = current_section;
                    if (filename.find("root") != std::string::npos) filename = "root";
                    filename += ".txt";

                    fs::path file_path = config_dir / filename;
                    std::ofstream out_file(file_path.string(), std::ios::binary);
                    if (out_file) {
                        out_file.write(current_data.c_str(), current_data.size());
                        out_file.close();
                        file_count++;
                        logger.log("Saved config file: " + file_path.string());
                    } else {
                        std::cerr << "ParseToStringWithFiles: Failed to create config file: " << file_path.string() << std::endl;
                    }

                    // Extract configuration GUID from root file if found
                    if (current_section.find("root") != std::string::npos) {
                        std::smatch match;
                        if (std::regex_search(current_data, match, config_guid_regex)) {
                            config_guid = match[1].str();
                            std::transform(config_guid.begin(), config_guid.end(), config_guid.begin(), ::tolower);
                            logger.log("Found configuration GUID: " + config_guid);
                        }
                    }

                    // Analyze metadata file to find object type mappings
                    if (current_section == "metadata" && !config_guid.empty()) {
                        std::smatch match;
                        auto search_start = current_data.cbegin();
                        while (std::regex_search(search_start, current_data.cend(), match, metadata_section_regex)) {
                            std::string metadata_guid = match[1].str();
                            std::transform(metadata_guid.begin(), metadata_guid.end(), metadata_guid.begin(), ::tolower);

                            auto type_it = guid_to_type.find(metadata_guid);
                            if (type_it != guid_to_type.end()) {
                                std::string metadata_objects = match[3].str();
                                // Extract object GUIDs from the section
                                std::regex object_guid_regex(R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");
                                std::sregex_iterator obj_it(metadata_objects.begin(), metadata_objects.end(), object_guid_regex);
                                std::sregex_iterator obj_end;
                                for (; obj_it != obj_end; ++obj_it) {
                                    std::string obj_guid = obj_it->str();
                                    std::transform(obj_guid.begin(), obj_guid.end(), obj_guid.begin(), ::tolower);
                                    type_to_objects[type_it->second].push_back(obj_guid);
                                    logger.log("Mapped object " + obj_guid + " to type " + type_it->second);
                                }
                            }
                            search_start = match.suffix().first;
                        }
                    }

                    current_section = "";
                    current_data = "";
                    continue;
                }

                // Determine metadata type for other files
                std::string metadata_type = "Неизвестные_объекты";

                // First try to match by object GUID in filename (if filename is GUID)
                std::string potential_guid = current_section;
                std::transform(potential_guid.begin(), potential_guid.end(), potential_guid.begin(), ::tolower);
                if (potential_guid.find('{') == std::string::npos && potential_guid.find('}') == std::string::npos) {
                    // Check if filename is a GUID
                    std::regex is_guid_pattern(R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");
                    if (std::regex_match(potential_guid, is_guid_pattern)) {
                        // Find which type this object belongs to
                        for (const auto& type_pair : type_to_objects) {
                            auto obj_it = std::find(type_pair.second.begin(), type_pair.second.end(), potential_guid);
                            if (obj_it != type_pair.second.end()) {
                                metadata_type = type_pair.first;
                                break;
                            }
                        }
                    }
                }

                // Fallback: search for GUIDs in content and use first matching metadata type
                if (metadata_type == "Неизвестные_объекты") {
                    auto guids = find_guids(current_data);
                    for (const auto& guid : guids) {
                        auto type_it = guid_to_type.find(guid);
                        if (type_it != guid_to_type.end()) {
                            metadata_type = type_it->second;
                            break;
                        }
                    }
                }

                // Create subdirectory for metadata type
                fs::path type_dir = base_dir / metadata_type;
                if (!ensure_directory(type_dir)) {
                    std::cerr << "ParseToStringWithFiles: Failed to create type directory: " << type_dir.string() << std::endl;
                    continue;
                }

                // Create filename based on section name (clean it up)
                std::string filename = current_section;
                // Remove illegal filename characters
                std::replace(filename.begin(), filename.end(), '"', '_');
                std::replace(filename.begin(), filename.end(), '/', '_');
                std::replace(filename.begin(), filename.end(), '\\', '_');
                std::replace(filename.begin(), filename.end(), ':', '_');
                std::replace(filename.begin(), filename.end(), '*', '_');
                std::replace(filename.begin(), filename.end(), '?', '_');
                std::replace(filename.begin(), filename.end(), '<', '_');
                std::replace(filename.begin(), filename.end(), '>', '_');
                std::replace(filename.begin(), filename.end(), '|', '_');
                std::replace(filename.begin(), filename.end(), '{', '_');
                std::replace(filename.begin(), filename.end(), '}', '_');

                // Add file extension
                if (filename != current_section) { // If we cleaned something
                    filename += ".txt";
                } else {
                    filename += ".txt";
                }

                // Save file
                fs::path file_path = type_dir / filename;
                std::ofstream out_file(file_path.string(), std::ios::binary);
                if (out_file) {
                    out_file.write(current_data.c_str(), current_data.size());
                    out_file.close();
                    file_count++;

                    logger.log("Saved file: " + file_path.string() + " (type: " + metadata_type + ")");
                } else {
                    std::cerr << "ParseToStringWithFiles: Failed to create file: " << file_path.string() << std::endl;
                }
            }

            // Start new section
            size_t start_pos = line.find("--- ") + 4;
            size_t end_pos = line.find(" ---");
            if (end_pos > start_pos) {
                current_section = line.substr(start_pos, end_pos - start_pos);
            } else {
                current_section = line.substr(start_pos);
            }
            current_data.clear();
        } else {
            // Add data to current section
            if (!current_data.empty()) {
                current_data += "\n";
            }
            current_data += line;
        }
    }

    // Save the last section
    if (!current_section.empty() && !current_data.empty()) {
        // Check if this is the configuration file
        if (current_section.find("root") != std::string::npos ||
            current_section == "version" ||
            current_section == "versions" ||
            current_section == "metadata") {

            // Save configuration files directly
            std::string filename = current_section;
            if (filename.find("root") != std::string::npos) filename = "root";
            filename += ".txt";

            fs::path file_path = config_dir / filename;
            std::ofstream out_file(file_path.string(), std::ios::binary);
            if (out_file) {
                out_file.write(current_data.c_str(), current_data.size());
                out_file.close();
                file_count++;
                logger.log("Saved config file: " + file_path.string());
            } else {
                std::cerr << "ParseToStringWithFiles: Failed to create config file: " << file_path.string() << std::endl;
            }
        } else {
            // Determine metadata type
            std::string metadata_type = "Неизвестные_объекты";

            // Check if filename is a GUID and find its type
            std::string potential_guid = current_section;
            std::transform(potential_guid.begin(), potential_guid.end(), potential_guid.begin(), ::tolower);
            if (potential_guid.find('{') == std::string::npos && potential_guid.find('}') == std::string::npos) {
                std::regex is_guid_pattern(R"([0-9a-fA-F]{8}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{4}-[0-9a-fA-F]{12})");
                if (std::regex_match(potential_guid, is_guid_pattern)) {
                    for (const auto& type_pair : type_to_objects) {
                        auto obj_it = std::find(type_pair.second.begin(), type_pair.second.end(), potential_guid);
                        if (obj_it != type_pair.second.end()) {
                            metadata_type = type_pair.first;
                            break;
                        }
                    }
                }
            }

            // Fallback to content-based matching
            if (metadata_type == "Неизвестные_объекты") {
                auto guids = find_guids(current_data);
                for (const auto& guid : guids) {
                    auto type_it = guid_to_type.find(guid);
                    if (type_it != guid_to_type.end()) {
                        metadata_type = type_it->second;
                        break;
                    }
                }
            }

            // Create directory and save file
            fs::path type_dir = base_dir / metadata_type;
            if (!ensure_directory(type_dir)) {
                std::cerr << "ParseToStringWithFiles: Failed to create type directory: " << type_dir.string() << std::endl;
            } else {
                std::string filename = current_section;
                std::replace(filename.begin(), filename.end(), '"', '_');
                std::replace(filename.begin(), filename.end(), '/', '_');
                std::replace(filename.begin(), filename.end(), '\\', '_');
                std::replace(filename.begin(), filename.end(), ':', '_');
                std::replace(filename.begin(), filename.end(), '*', '_');
                std::replace(filename.begin(), filename.end(), '?', '_');
                std::replace(filename.begin(), filename.end(), '<', '_');
                std::replace(filename.begin(), filename.end(), '>', '_');
                std::replace(filename.begin(), filename.end(), '|', '_');
                std::replace(filename.begin(), filename.end(), '{', '_');
                std::replace(filename.begin(), filename.end(), '}', '_');
                filename += ".txt";

                fs::path file_path = type_dir / filename;
                std::ofstream out_file(file_path.string(), std::ios::binary);
                if (out_file) {
                    out_file.write(current_data.c_str(), current_data.size());
                    out_file.close();
                    file_count++;

                    logger.log("Saved file: " + file_path.string() + " (type: " + metadata_type + ")");
                } else {
                    std::cerr << "ParseToStringWithFiles: Failed to create file: " << file_path.string() << std::endl;
                }
            }
        }
    }

    logger.log("ParseToStringWithFiles completed. Saved " + std::to_string(file_count) + " files.");
    cout << "ParseToStringWithFiles: Saved " << file_count << " files in organized structure by metadata types." << endl;

    return V8UNPACK_OK;
}

} // namespace v8unpack
