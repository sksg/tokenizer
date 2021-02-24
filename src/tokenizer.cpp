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

struct token {
    enum {
        punctuation_kind,
        number_kind,
        symbol_kind,
        bad_char_kind
    } kind;

    const char* const position;
    int length;
    int column, column_length;
};

void print_token(const token &tkn) {
    switch (tkn.kind)
    {
    case token::punctuation_kind:
        std::cout << "PCT";
        break;
    case token::number_kind:
        std::cout << "NUM";
        break;
    case token::symbol_kind:
        std::cout << "SYM";
        break;
    case token::bad_char_kind:
        std::cout << "BAD";
        break;
    }
    std::cout << " @ col:" << tkn.column;
    if (tkn.column_length > 1)
        std::cout << ", len:" << tkn.column_length;
    auto tkn_str = std::string_view(tkn.position, tkn.length);
    std::cout << " :: '" << tkn_str << "'" << std::endl;
}

struct scanner {
    const char* const begin_position;
    const char* current_position;
    int current_char_length;
    int current_column;
    const char* const end_position;

    static scanner from_string(const std::string str) {
        return {str.data(), str.data(), 1, 1, str.data() + str.length()};
    }

    bool not_at_end() {
        return current_position < end_position;
    }
    
    char current_char(int offset = 0) {
        return current_position[offset];
    }
    
    void consume_current() {
        current_position += current_char_length;
        current_char_length = 1;
        current_column++;
    }

    int remaining_length() {
        return end_position - current_position;
    }
    
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
bool consume_good_utf8(scanner &scr, const uint32_t (&unicode_table)[N][2]) {
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

    if (!(scr.current_char() & UTF8_MULTIBYTE_MASK))
        return false;

    uint32_t code = 0;
    if ((scr.current_char() & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
        if (scr.remaining_length() < 4 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (scr.current_char(0) & UTF8_4BYTE_MASK_INV) << 18;
        uint32_t b1 = (scr.current_char(1) & UTF8_NEXT_BYTE_MASK) << 12;
        uint32_t b2 = (scr.current_char(2) & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b3 = (scr.current_char(3) & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2 | b3;
        scr.current_char_length = 4;
    }
    else if ((scr.current_char() & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
        if (scr.remaining_length() < 3 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (scr.current_char(0) & UTF8_3BYTE_MASK_INV) << 12;
        uint32_t b1 = (scr.current_char(1) & UTF8_NEXT_BYTE_MASK) << 6;
        uint32_t b2 = (scr.current_char(2) & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1 | b2;
        scr.current_char_length = 3;
    }
    else if ((scr.current_char() & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
        if (scr.remaining_length() < 2 ) {
            std::cerr << "UTF8 string error (missing bytes)" << std::endl;
        }
        uint32_t b0 = (scr.current_char(0) & UTF8_2BYTE_MASK_INV) << 6;
        uint32_t b1 = (scr.current_char(1) & UTF8_NEXT_BYTE_MASK) << 0;
        code = b0 | b1;
        scr.current_char_length = 2;
    }
    else {
        std::cerr << "ERROR: Bad UTF8 code point '" << scr.current_char() << "'" << std::endl;
    }
    
    for (size_t i = 0; i < N; i++) {
        if (code >= unicode_table[i][0] && code <= unicode_table[i][1]) {
            scr.consume_current();
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
        auto scr = scanner::from_string(line);
        while (scr.not_at_end()) {

            token current_token = {
                token::bad_char_kind,
                scr.current_position, 1,
                scr.current_column, 1
            };
    
            // Single (byte) punctuation characters
            switch (scr.current_char()) {
                case ' ':
                case '\t':
                    scr.consume_current();
                    continue;
                case '(':
                case ')':
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                    current_token.kind = token::punctuation_kind;
                    print_token(current_token);
                    scr.consume_current();
                    continue;
            }

            // Number literal
            while (scr.consume_numeric());
            // Number literal, decimal/fractional part
            if (scr.current_char() == '.') {
                scr.consume_current();
                while (scr.consume_numeric());
            }
            
            if (current_token.position != scr.current_position) {
                // Number literal has been parsed
                current_token.kind = token::number_kind;
                current_token.length = scr.current_position - current_token.position;
                current_token.column_length = scr.current_column - current_token.column;
                print_token(current_token);
                continue;
            }

            // identifier
            scr.current_char_length = 1;
            if (scr.consume_alpha() || consume_good_utf8(scr, id_start_table))
                while (
                    scr.consume_alpha() ||
                    scr.consume_numeric() ||
                    consume_good_utf8(scr, id_continue_table)
                );

            if (current_token.position != scr.current_position) {
                // Identifier has been parsed
                current_token.kind = token::symbol_kind;
                current_token.length = scr.current_position - current_token.position;
                current_token.column_length = scr.current_column - current_token.column;
                print_token(current_token);
                continue;
            }

            current_token.kind = token::bad_char_kind;
            current_token.length = scr.current_char_length;
            print_token(current_token);
            scr.consume_current();
        };

        std::cout << "EOL" << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

