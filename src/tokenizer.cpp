#include <iostream>
#include <string>
#include <sstream>
#include <string_view>
#include <cstdint>

#include "unicode_tables.h"

// Test strings:
// a = 1
// b=2
// c =3
// a+b * c

template <int N>
size_t read_in_unicode_table(
    const std::string_view &line,
    const uint32_t (&unicode_table)[N][2]
) {
    // multibyte test: 0b1xxxxxxx;
    static const uint8_t UTF8_MULTIBYTE_MASK = 0x80;  // 0b10000000;

    // 4-byte test: 0b11110xxx;
    static const uint8_t UTF8_4BYTE_MASK = 0xf8;  // 0b11111000;
    static const uint8_t UTF8_4BYTE_MASK_INV = 0x07;  // 0b00000111;
    static const uint8_t UTF8_4BYTE_TEST = 0xf0;  // 0b11110000;

    // 3-byte test: 0b1110xxxx;
    static const uint8_t UTF8_3BYTE_MASK = 0xf0;  // 0b11110000;
    static const uint8_t UTF8_3BYTE_MASK_INV = 0x0f;  // 0b00001111;
    static const uint8_t UTF8_3BYTE_TEST = 0xe0;  // 0b11100000;

    // 2-byte test: 0b110xxxxx;
    static const uint8_t UTF8_2BYTE_MASK = 0xe0;  // 0b11100000;
    static const uint8_t UTF8_2BYTE_MASK_INV = 0x1f;  // 0b00001111;
    static const uint8_t UTF8_2BYTE_TEST = 0xc0;  // 0b11000000;

    static const uint8_t UTF8_NEXT_BYTE_MASK = 0x3f;  // 0b00111111;

    if (!(line[0] & UTF8_MULTIBYTE_MASK))
        return 0;

    uint32_t code = 0;
    size_t code_len = 0;
    if ((line[0] & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
        if (4 > line.length()) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (line[0] & UTF8_4BYTE_MASK_INV) << 18;
        uint32_t b1 = (line[1] & UTF8_NEXT_BYTE_MASK) << 12;
        uint32_t b2 = (line[2] & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b3 = (line[3] & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2 | b3;
        code_len = 4;
    }
    else if ((line[0] & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
        if (3 > line.length()) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (line[0] & UTF8_3BYTE_MASK_INV) << 12;
        uint32_t b1 = (line[1] & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b2 = (line[2] & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2;
        code_len = 3;
    }
    else if ((line[0] & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
        if (2 > line.length()) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (line[0] & UTF8_2BYTE_MASK_INV) << 6;
        uint32_t b1 = (line[1] & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1;
        code_len = 2;
    }
    else {
        std::cerr << "Unknown unicode character '" << line[0] << "'";
        std::cerr << "(\\U" << (uint)(uint8_t)line[0] << ")" << std::endl;
    }
    
    for (size_t i = 0; i < N; i++)
        if (code >= unicode_table[i][0] && code <= unicode_table[i][1])
            return code_len;

    return 0;
}

int main() {
    auto short_welcome = "Welcome to the tokenizer.";
    auto long_welcome = 
        "Input a line of code, and the tokenizer will return the tokens. "
        "Exit by closing input stream e.g. ctrl+d (unix) or ctrl+z (win).";
    auto prompt = "tokenizer> ";
    
    std::cout << short_welcome << std::endl;
    std::cout << long_welcome << std::endl;
    std::cout << std::endl << prompt << std::flush;

    
    for ( // Infinite REPL loop
        std::string next_line;
        std::getline(std::cin, next_line);
        std::cout << prompt << std::flush
    ) {
        size_t id_start = 0;
        size_t position = 0;
        while (position < next_line.length()) {
            char current_char = next_line[position];
    
            // Single (byte) punctuation characters
            switch (current_char) {
                case ' ':
                case '\t':
                    continue; // gooble, gooble
                case '(':
                case ')':
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                    std::cout << "PCT @ col:" << position + 1;
                    std::cout << " :: '" << current_char << "'" << std::endl;
                    continue;
            }

            // Number literal
            id_start = position;
            if (current_char >= '0' && current_char <= '9') {
                position++;
                while (position < next_line.length()) {
                    current_char = next_line[position];
                    if (!(current_char >= '0' && current_char <= '9')) {
                        break;
                    }
                    position++;
                };
            }
            // Number literal, decimal/fractional part
            if (current_char == '.') {
                position++;
                while (position < next_line.length()) {
                    current_char = next_line[position];
                    if (!(current_char >= '0' && current_char <= '9')) {
                        break;
                    }
                    position++;
                };
            }
            
            if (id_start != position) { // Number literal has been parsed
                auto length = position - id_start;
                auto number = next_line.substr(id_start, length);
                std::cout << "NUM @ col:" << id_start + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                continue;
            }

            bool is_id_start = (
                (current_char >= 'a' && current_char <= 'z') ||
                (current_char >= 'A' && current_char <= 'Z') ||
                (current_char == '_')
            );
            if (is_id_start)
                position++;
            else {
                auto remaining_line = std::string_view(next_line);
                remaining_line.remove_prefix(position);
                auto code_len = read_in_unicode_table(
                    remaining_line,
                    id_start_table
                );
                if (code_len > 0) {
                    is_id_start = true;
                    position += code_len;
                }
            }

            if (is_id_start) {
                // identifier
                bool is_id_continue = true;
                while (is_id_continue && position < next_line.length()) {
                    current_char = next_line[position];
                    bool is_id_continue = (
                        (current_char >= 'a' && current_char <= 'z') ||
                        (current_char >= 'A' && current_char <= 'Z') ||
                        (current_char >= '0' && current_char <= '9') ||
                        (current_char == '_')
                    );
                    if (is_id_start)
                        position++;
                    else {
                        auto remaining_line = std::string_view(next_line);
                        remaining_line.remove_prefix(position);
                        auto code_len = read_in_unicode_table(
                            remaining_line,
                            id_start_table
                        );
                        if (code_len > 0) {
                            is_id_start = true;
                            position += code_len;
                        }
                    }
                };
                current_char = next_line[position];
            }

            if (id_start != position) { // Identifier has been parsed
                auto length = position - id_start;
                auto number = next_line.substr(id_start, length);
                std::cout << "SYM @ col:" << id_start + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                continue;
            }

            std::cerr << "Unknown character '" << current_char << "'" << std::endl;
            position++;
        }
        std::cout << "EOL" << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

