/*----------------------------------------------------------
This Source Code Form is subject to the terms of the
Mozilla Public License, v.2.0. If a copy of the MPL
was not distributed with this file, You can obtain one
at http://mozilla.org/MPL/2.0/.
----------------------------------------------------------*/

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <cstring>
#include <stdexcept>
#include "TMSTree.h"

class ParserStr {
private:
    std::string buffer_;
    size_t buffer_size_;
    size_t used_size_;
    char* current_symbol_;

    void CheckedMovePointer(char ch) {
        if (used_size_ >= buffer_size_) {
            buffer_size_ = (buffer_size_ == 0) ? 256 : buffer_size_ * 2;
            buffer_.resize(buffer_size_);
            current_symbol_ = &buffer_[used_size_];
        }
        *current_symbol_++ = ch;
        used_size_++;
    }

    void ResetField() {
        used_size_ = 0;
        current_symbol_ = &buffer_[0];
        std::fill(buffer_.begin(), buffer_.end(), 0);
    }

    void AddMessage(const std::string& message) {
        // Реализация логирования сообщений об ошибках
        throw std::runtime_error(message);
    }

public:
    std::vector<std::string> ParseString(const std::string& str) {
        
        std::vector<std::string> result;

        // Trim spaces from start and end
        size_t start = str.find_first_not_of(" \t\n\r");
        size_t end = str.find_last_not_of(" \t\n\r");

        if (start == std::string::npos || end == std::string::npos) {
            return result; // empty string
        }

        std::string trimmed_str = str.substr(start, end - start + 1);

        if (trimmed_str.empty()) {
            return result;
        }

        TMetaParserStates state = TMetaParserStates::mpsIdle;
        buffer_size_ = 256;
        buffer_.resize(buffer_size_, 0);
        ResetField();

        auto is_hex_digit = [](char c) -> bool {
            return (c >= '0' && c <= '9') ||
                (c >= 'a' && c <= 'f') ||
                (c >= 'A' && c <= 'F');
            };

        auto is_base64_char = [](char c) -> bool {
            return (c >= 'a' && c <= 'z') ||
                (c >= 'A' && c <= 'Z') ||
                (c >= '0' && c <= '9') ||
                c == '+' || c == '/' || c == '=';
            };

        try {
            for (char cur_char : trimmed_str) {
                switch (state) {
                case TMetaParserStates::mpsIdle:
                    switch (cur_char) {
                    case ',':
                        result.emplace_back("");
                        break;
                    default:
                        if (is_hex_digit(cur_char)) {
                            state = TMetaParserStates::mpsReadNumField;
                            CheckedMovePointer(cur_char);
                        }
                        else if (cur_char == '\"') {
                            state = TMetaParserStates::mpsReadStrField;
                            ResetField();
                        }
                        else if (is_base64_char(cur_char)) {
                            state = TMetaParserStates::mpsReadBase64;
                            CheckedMovePointer(cur_char);
                        }
                        else {
                            state = TMetaParserStates::mpsError;
                        }
                        break;
                    }
                    break;

                case TMetaParserStates::mpsReadNumField:
                    if (cur_char == ',') {
                        result.emplace_back(buffer_.data());
                        ResetField();
                        state = TMetaParserStates::mpsIdle;
                    }
                    else if (is_hex_digit(cur_char) || cur_char == '-') {
                        CheckedMovePointer(cur_char);
                    }
                    else if (is_base64_char(cur_char)) {
                        state = TMetaParserStates::mpsReadBase64;
                        CheckedMovePointer(cur_char);
                    }
                    else {
                        state = TMetaParserStates::mpsError;
                    }
                    break;

                case TMetaParserStates::mpsReadStrField:
                    if (cur_char == '\"') {
                        state = TMetaParserStates::mpsFoundQuote;
                    }
                    else if (cur_char == ',') {
                        state = TMetaParserStates::mpsError;
                    }
                    else {
                        CheckedMovePointer(cur_char);
                        state = TMetaParserStates::mpsReadQuotedField;
                    }
                    break;

                case TMetaParserStates::mpsReadBase64:
                    if (cur_char == ',') {
                        state = TMetaParserStates::mpsIdle;
                    }
                    else {
                        CheckedMovePointer(cur_char);
                    }
                    break;

                case TMetaParserStates::mpsReadQuotedField:
                    if (cur_char == '\"') {
                        state = TMetaParserStates::mpsFoundQuote;
                    }
                    else {
                        CheckedMovePointer(cur_char);
                    }
                    break;

                case TMetaParserStates::mpsFoundQuote:
                    if (cur_char == '\"') {
                        state = TMetaParserStates::mpsReadQuotedSubStr;
                        CheckedMovePointer('\"');
                    }
                    else if (cur_char == ',') {
                        state = TMetaParserStates::mpsIdle;
                        result.emplace_back(buffer_.data());
                        ResetField();
                        // если заканчивается пустым параметром
                        if (&cur_char == &trimmed_str.back()) {
                            result.emplace_back("");
                        }
                    }
                    else {
                        state = TMetaParserStates::mpsError;
                    }
                    break;

                case TMetaParserStates::mpsReadQuotedSubStr:
                    if (cur_char == '\"') {
                        state = TMetaParserStates::mpsWaitForQuote;
                    }
                    else {
                        CheckedMovePointer(cur_char);
                    }
                    break;

                case TMetaParserStates::mpsWaitForQuote:
                    if (cur_char == '\"') {
                        state = TMetaParserStates::mpsReadQuotedField;
                        CheckedMovePointer('\"');
                    }
                    else {
                        state = TMetaParserStates::mpsError;
                    }
                    break;

                default:
                    break;
                }

                if (state == TMetaParserStates::mpsError) {
                    AddMessage("Неверный символ в потоке: [" + std::string(1, cur_char) + "] [" + trimmed_str + "]");
                    break;
                }
            }

            // Добавляем последнее поле если парсер в подходящем состоянии
            if (state == TMetaParserStates::mpsReadQuotedField ||
                state == TMetaParserStates::mpsReadNumField ||
                state == TMetaParserStates::mpsReadBase64 ||
                state == TMetaParserStates::mpsFoundQuote) {
                result.emplace_back(buffer_.data());
            }

        }
        catch (...) {
            // Обработка исключений
            throw;
        }

        return result;
    }

    ParserStr() = default;
};

bool SameText(const std::string& str1, const std::string& str2);

