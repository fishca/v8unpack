#include "ErrorCodes.h"

#include <sstream>

namespace v8unpack {

// ===== Реализация ErrorInfo =====

ErrorInfo::ErrorInfo() : m_code(ErrorCode::SUCCESS) {}

ErrorInfo::ErrorInfo(ErrorCode code) : m_code(code) {
    m_message = getDefaultMessage(code);
}

ErrorInfo::ErrorInfo(ErrorCode code, const std::string& message)
    : m_code(code), m_message(message) {
    if (m_message.empty()) {
        m_message = getDefaultMessage(code);
    }
}

ErrorInfo::ErrorInfo(ErrorCode code, const std::string& message,
                    const std::string& details, const std::string& context)
    : m_code(code), m_message(message), m_details(details), m_context(context) {
    if (m_message.empty()) {
        m_message = getDefaultMessage(code);
    }
}

bool ErrorInfo::isSuccess() const {
    return m_code == ErrorCode::SUCCESS;
}

bool ErrorInfo::isFileError() const {
    int codeValue = static_cast<int>(m_code);
    return codeValue >= 100 && codeValue < 200;
}

bool ErrorInfo::isMetadataError() const {
    int codeValue = static_cast<int>(m_code);
    return codeValue >= 400 && codeValue < 500;
}

ErrorCode ErrorInfo::code() const {
    return m_code;
}

const std::string& ErrorInfo::message() const {
    return m_message;
}

const std::string& ErrorInfo::details() const {
    return m_details;
}

const std::string& ErrorInfo::context() const {
    return m_context;
}

int ErrorInfo::toInt(ErrorCode code) {
    return static_cast<int>(code);
}

ErrorCode ErrorInfo::fromInt(int value) {
    if (value < 0 || value > static_cast<int>(ErrorCode::UNKNOWN_ERROR)) {
        return ErrorCode::UNKNOWN_ERROR;
    }
    return static_cast<ErrorCode>(value);
}

std::string ErrorInfo::getDefaultMessage(ErrorCode code) {
    switch (code) {
        case ErrorCode::SUCCESS:
            return "Operation completed successfully";

        // Ошибки аргументов (1-99)
        case ErrorCode::INVALID_ARGUMENTS:
            return "Invalid command line arguments";
        case ErrorCode::MISSING_REQUIRED_ARG:
            return "Missing required argument";
        case ErrorCode::INVALID_COMMAND:
            return "Invalid or unrecognized command";
        case ErrorCode::FILE_NOT_FOUND:
            return "File not found";
        case ErrorCode::DIRECTORY_NOT_FOUND:
            return "Directory not found";
        case ErrorCode::INVALID_FILE_PATH:
            return "Invalid file path";

        // Ошибки файлов (100-199)
        case ErrorCode::FILE_READ_ERROR:
            return "Failed to read file";
        case ErrorCode::FILE_WRITE_ERROR:
            return "Failed to write file";
        case ErrorCode::FILE_ACCESS_DENIED:
            return "Access denied";
        case ErrorCode::INSUFFICIENT_DISK_SPACE:
            return "Insufficient disk space";
        case ErrorCode::FILE_CORRUPTED:
            return "File is corrupted";

        // Ошибки V8 формата (200-299)
        case ErrorCode::INVALID_V8_FORMAT:
            return "Invalid V8 file format";
        case ErrorCode::MISSING_V8_SIGNATURE:
            return "Missing V8 file signature";
        case ErrorCode::V8_VERSION_MISMATCH:
            return "V8 version mismatch";
        case ErrorCode::METADATA_CORRUPTED:
            return "V8 metadata is corrupted";

        // Ошибки сжатия (300-399)
        case ErrorCode::COMPRESSION_FAILED:
            return "Compression failed";
        case ErrorCode::DECOMPRESSION_FAILED:
            return "Decompression failed";
        case ErrorCode::UNSUPPORTED_COMPRESSION:
            return "Unsupported compression method";

        // Ошибки метаданных (400-499)
        case ErrorCode::METADATA_PARSING_ERROR:
            return "Failed to parse metadata";
        case ErrorCode::INVALID_ROOT_STRUCTURE:
            return "Invalid root metadata structure";
        case ErrorCode::MISSING_CONFIGURATION_GUID:
            return "Missing configuration GUID";
        case ErrorCode::METADATA_SERIALIZATION_ERROR:
            return "Failed to serialize metadata";

        // Ошибки команд (500-599)
        case ErrorCode::COMMAND_EXECUTION_FAILED:
            return "Command execution failed";
        case ErrorCode::COMMAND_TIMEOUT:
            return "Command execution timeout";
        case ErrorCode::COMMAND_CANCELLED:
            return "Command execution cancelled";

        // Системные ошибки (600-699)
        case ErrorCode::OUT_OF_MEMORY:
            return "Out of memory";
        case ErrorCode::SYSTEM_CALL_FAILED:
            return "System call failed";

        case ErrorCode::UNKNOWN_ERROR:
        default:
            return "Unknown error occurred";
    }
}

ErrorInfo::operator bool() const {
    return isSuccess();
}

// ===== Реализация ErrorUtils =====

ErrorInfo ErrorUtils::createFileError(const std::string& filename,
                                    const std::string& operation,
                                    const std::string& additionalInfo) {
    std::string message = "File operation '" + operation + "' failed for '" + filename + "'";
    std::string details;
    ErrorCode code;

    // Определяем наиболее подходящий код ошибки на основе operation
    if (operation.find("read") != std::string::npos || operation.find("open") != std::string::npos) {
        code = ErrorCode::FILE_READ_ERROR;
        details = "Cannot access file content";
    } else if (operation.find("write") != std::string::npos || operation.find("create") != std::string::npos) {
        code = ErrorCode::FILE_WRITE_ERROR;
        details = "Cannot modify file";
    } else {
        code = ErrorCode::FILE_ACCESS_DENIED;
        details = "File access denied";
    }

    if (!additionalInfo.empty()) {
        details += " (" + additionalInfo + ")";
    }

    return ErrorInfo(code, message, details, filename);
}

ErrorInfo ErrorUtils::createArgumentError(const std::string& argument,
                                       const std::string& expected,
                                       const std::string& actual) {
    std::string message = "Invalid argument: " + argument;

    std::ostringstream detailsStream;
    detailsStream << "Expected: '" << expected << "', but got: '" << actual << "'";

    return ErrorInfo(ErrorCode::INVALID_ARGUMENTS, message, detailsStream.str());
}

ErrorInfo ErrorUtils::createV8FormatError(const std::string& issue,
                                       const std::string& filename) {
    std::string message = "V8 format error";
    std::string details = issue;

    return ErrorInfo(ErrorCode::INVALID_V8_FORMAT, message, details, filename);
}

} // namespace v8unpack
