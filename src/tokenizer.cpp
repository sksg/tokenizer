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

struct stream {
    const char* const begin_position;
    const char* current_position;
    int current_char_length;
    const char* const end_position;

    static stream from_string(const std::string str) {
        return {str.data(), str.data(), 1, str.data() + str.length()};
    }

    bool not_at_end() { return current_position < end_position; }
    char current_char(int offset = 0) { return current_position[offset]; }
    void consume_current() { current_position += current_char_length; current_char_length = 1; }
    int remaining_length() { return end_position - current_position; }
    
    bool consume_numeric() {
        if (current_char() >= '0' && current_char() <= '9') {
            consume_current();
            return true;
        }
        return false;
    }

    bool consume_alpha() {
        if (
                (current_char() >= 'a' && current_char() <= 'z') ||
                (current_char() >= 'A' && current_char() <= 'Z') ||
                (current_char() == '_')
        ) {
            consume_current();
            return true;
        }
        return false;
    }
};

template <int N>
bool consume_good_utf8(stream &str, const uint32_t (&unicode_table)[N][2]) {
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

    if (!(str.current_char() & UTF8_MULTIBYTE_MASK))
        return false;

    uint32_t code = 0;
    if ((str.current_char() & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
        if (str.remaining_length() < 4 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (str.current_char(0) & UTF8_4BYTE_MASK_INV) << 18;
        uint32_t b1 = (str.current_char(1) & UTF8_NEXT_BYTE_MASK) << 12;
        uint32_t b2 = (str.current_char(2) & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b3 = (str.current_char(3) & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2 | b3;
        str.current_char_length = 4;
    }
    else if ((str.current_char() & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
        if (str.remaining_length() < 3 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (str.current_char(0) & UTF8_3BYTE_MASK_INV) << 12;
        uint32_t b1 = (str.current_char(1) & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b2 = (str.current_char(2) & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2;
        str.current_char_length = 3;
    }
    else if ((str.current_char() & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
        if (str.remaining_length() < 2 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (str.current_char(0) & UTF8_2BYTE_MASK_INV) << 6;
        uint32_t b1 = (str.current_char(1) & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1;
        str.current_char_length = 2;
    }
    else {
        std::cerr << "ERROR: Bad UTF8 code point '" << str.current_char() << "'" << std::endl;
    }
    
    for (size_t i = 0; i < N; i++) {
        if (code >= unicode_table[i][0] && code <= unicode_table[i][1]) {
            str.consume_current();
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
        std::string line;
        std::getline(std::cin, line);
        std::cout << prompt << std::flush
    ) {
        auto str = stream::from_string(line);
        while (str.not_at_end()) {
    
            // Single (byte) punctuation characters
            switch (str.current_char()) {
                case ' ':
                case '\t':
                    str.consume_current();
                    continue;
                case '(':
                case ')':
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                    std::cout << "PCT @ col:" << int(str.current_position - str.begin_position) + 1;
                    std::cout << " :: '" << str.current_char() << "'" << std::endl;
                    str.consume_current();
                    continue;
            }

            // Number literal
            auto id_start = str.current_position;
            while (str.consume_numeric());
            // Number literal, decimal/fractional part
            if (str.current_char() == '.') {
                str.consume_current();
                while (str.consume_numeric());
            }
            
            if (id_start != str.current_position) { // Number literal has been parsed
                int length = str.current_position - id_start;
                auto number = std::string_view(id_start, length);
                std::cout << "NUM @ col:" << int(id_start - str.begin_position) + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                continue;
            }

            // identifier
            str.current_char_length = 1;
            if (str.consume_alpha() || consume_good_utf8(str, id_start_table))
                while (str.consume_alpha() || consume_good_utf8(str, id_continue_table));

            if (id_start != str.current_position) { // Identifier has been parsed
                int length = str.current_position - id_start;
                auto number = std::string_view(id_start, length);
                std::cout << "SYM @ col:" << int(id_start - str.begin_position) + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                continue;
            }

            std::cerr << "Unknown character '" << std::string_view(str.current_position, str.current_char_length) << "'" << std::endl;
            str.consume_current();
        };

        std::cout << "EOL" << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

