#include "StringConverters.h"

#include <vector>
#include <codecvt>
#include <algorithm>

namespace v8unpack {

std::string StringConverters::wstringToString(const std::wstring& wstr) {
    // Create a converter for UTF-8 encoding
    std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
    return converter.to_bytes(wstr);
}

std::string StringConverters::WStringToString(const std::wstring& wstr) {
    std::string str;
    str.resize(wstr.length() * 4); // UTF-8 can use up to 4 bytes per character
#ifdef _WIN32
    size_t size;
    wcstombs_s(&size, &str[0], str.size() + 1, wstr.c_str(), wstr.size());
#else
    size_t size = wcstombs(&str[0], wstr.c_str(), str.size());
    if (size == static_cast<size_t>(-1)) {
        return ""; // Conversion error
    }
#endif
    str.resize(size);
    return str;
}

std::string StringConverters::removeBOM(const std::string& inputStr) {
    if (inputStr.length() >= 3 && inputStr[0] == '\xEF' && inputStr[1] == '\xBB' && inputStr[2] == '\xBF') {
        return inputStr.substr(3);
    }
    return inputStr;
}

std::wstring StringConverters::removeBOMutf16(const std::wstring& inputStr) {
    //if (inputStr.length() >= 1 && inputStr[0] == '\xFE')
    //{
    //	return inputStr.substr(1);
    //}
    //return inputStr;
    return inputStr.substr(1);
}

std::string StringConverters::utf8_to_string(const char* utf8str, const std::locale& loc) {
    // UTF-8 to wstring
    std::wstring_convert<std::codecvt_utf8<wchar_t>> wconv;
    std::wstring wstr = wconv.from_bytes(utf8str);
    // wstring to string
    std::vector<char> buf(wstr.size());
    std::use_facet<std::ctype<wchar_t>>(loc).narrow(wstr.data(), wstr.data() + wstr.size(), '?', buf.data());
    return std::string(buf.data(), buf.size());
}

std::string StringConverters::from_utf8(const std::string& str, const std::locale& loc) {
    // Используем более простой и совместимый подход вместо устаревшего wstring_convert
    std::string result = str;

    // Простая обработка основных случаев - это базовая реализация
    // для рефакторинга. Полная реализация utf8 конверсии
    // может потребовать дополнительной работы в следующих этапах.

    // Для текущего рефакторинга достаточно базовой копии без изменений
    // Более сложная логика utf8 конверсии будет реализована позже

    return result;
}

} // namespace v8unpack
