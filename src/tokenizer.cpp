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
bool read_utf8_id(
    char* (&current_char),
    int &code_length,
    const char* last_char,
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

    if (!(*current_char & UTF8_MULTIBYTE_MASK))
        return 0;

    uint32_t code = 0;
    int remaining_length = last_char - current_char;
    if ((*current_char & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
        if (remaining_length < 4 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (current_char[0] & UTF8_4BYTE_MASK_INV) << 18;
        uint32_t b1 = (current_char[1] & UTF8_NEXT_BYTE_MASK) << 12;
        uint32_t b2 = (current_char[2] & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b3 = (current_char[3] & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2 | b3;
        code_length = 4;
    }
    else if ((*current_char & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
        if (remaining_length < 3 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (current_char[0] & UTF8_3BYTE_MASK_INV) << 12;
        uint32_t b1 = (current_char[1] & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b2 = (current_char[2] & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2;
        code_length = 3;
    }
    else if ((*current_char & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
        if (remaining_length < 2 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (current_char[0] & UTF8_2BYTE_MASK_INV) << 6;
        uint32_t b1 = (current_char[1] & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1;
        code_length = 2;
    }
    else {
        std::cerr << "Unknown UTF8 character '" << *current_char << "'" << std::endl;
    }
    
    for (size_t i = 0; i < N; i++) {
        if (code >= unicode_table[i][0] && code <= unicode_table[i][1]) {
            current_char += code_length;
            return true;
        }
    }

    return false;
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
        std::string line_str;
        std::getline(std::cin, line_str);
        std::cout << prompt << std::flush
    ) {
        char* first_char = line_str.data();
        char* current_char = line_str.data();
        char* last_char = line_str.data() + line_str.length();
        while (current_char < last_char) {
    
            // Single (byte) punctuation characters
            switch (*current_char) {
                case ' ':
                case '\t':
                    current_char++;
                    continue; // gooble, gooble
                case '(':
                case ')':
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                    std::cout << "PCT @ col:" << int(current_char - first_char) + 1;
                    std::cout << " :: '" << *current_char << "'" << std::endl;
                    current_char++;
                    continue;
            }

            // Number literal
            auto id_start = current_char;
            if (*current_char >= '0' && *current_char <= '9') {
                current_char++;
                for (;current_char < last_char; current_char++) {
                    if (!(*current_char >= '0' && *current_char <= '9')) {
                        break;
                    }
                }
            }
            // Number literal, decimal/fractional part
            if (*current_char == '.') {
                current_char++;
                for (;current_char < last_char; current_char++) {
                    if (!(*current_char >= '0' && *current_char <= '9')) {
                        break;
                    }
                }
            }
            
            if (id_start != current_char) { // Number literal has been parsed
                int length = current_char - id_start;
                auto number = std::string_view(id_start, length);
                std::cout << "NUM @ col:" << int(id_start - first_char) + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                continue;
            }

            int code_length = 1;
            bool is_id_start = (
                (*current_char >= 'a' && *current_char <= 'z') ||
                (*current_char >= 'A' && *current_char <= 'Z') ||
                (*current_char == '_')
            );
            if (is_id_start)
                current_char++;
            else
                is_id_start = read_utf8_id(current_char, code_length, last_char, id_start_table);

            bool bad_utf8_char = false;
            if (is_id_start) {
                // identifier
                bool is_id_continue = true;
                while (is_id_continue && current_char < last_char) {
                    is_id_continue = (
                        (*current_char >= 'a' && *current_char <= 'z') ||
                        (*current_char >= 'A' && *current_char <= 'Z') ||
                        (*current_char >= '0' && *current_char <= '9') ||
                        (*current_char == '_')
                    );
                    if (is_id_continue)
                        current_char++;
                    else
                        is_id_continue = read_utf8_id(current_char, code_length, last_char, id_continue_table);
                };
            }

            if (id_start != current_char) { // Identifier has been parsed
                int length = current_char - id_start;
                auto number = std::string_view(id_start, length);
                std::cout << "SYM @ col:" << int(id_start - first_char) + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                continue;
            }

            std::cerr << "Unknown character '" << std::string_view(current_char, code_length) << "'" << std::endl;
            current_char += code_length;
        };

        std::cout << "EOL" << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

