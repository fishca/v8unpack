#ifndef V8_META_PARSER_H
#define V8_META_PARSER_H

#include <string>
#include <vector>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

namespace v8unpack {

// Legacy functions for GUID and object processing
std::vector<std::string> find_guids(const std::string& text);

std::string extract_object_name(const std::string& content, const std::string& expected_guid);

bool ensure_directory(const fs::path& path);

/**
 * @brief Распаковывание конфигурации 1C v8 в организованную файловую структуру по GUID метаданных
 * Функция анализирует результаты ParseToString и сохраняет данные в поддиректории по типам метаданных
 * @param config_string Входная строка с распакованными данными от ParseToString
 * @param dirname Каталог для сохранения файлов
 * @return Код возврата (0 - успешно, отрицательные значения - ошибки)
 */
int ParseToStringWithFiles(const std::string &config_string, const std::string &dirname);

} // namespace v8unpack

#endif // V8_META_PARSER_H
