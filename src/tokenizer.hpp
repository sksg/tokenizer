#include <iostream>
#include <string>
#include <string_view>
#include <cstdint>
#include <cassert>

#include "unicode_tables.h"

struct token {
    enum {
        punctuation_kind,
        number_kind,
        symbol_kind,
        eol_kind,
        bad_char_kind
    } kind = bad_char_kind;

    const char* position = nullptr;
    uint length = 0;

    // Optimization(sorgre): Column data can be determined when printing.
    uint column = 0;
    uint column_length = 0;

    std::string_view string_view() const {
        return std::string_view(position, length);
    }
};

std::ostream& operator << (std::ostream& str, const token &tkn) {
    assert(tkn.position != nullptr);
    assert(tkn.length != 0);
    assert(tkn.column != 0);
    assert(tkn.column_length != 0);

    switch (tkn.kind) {
        case token::punctuation_kind: str << "PCT"; break;
        case token::number_kind:      str << "NUM"; break;
        case token::symbol_kind:      str << "SYM"; break;
        case token::eol_kind:         str << "EOL"; break;
        case token::bad_char_kind:    str << "BAD"; break;
    }
    str << " @ col:" << tkn.column;
    if (tkn.column_length > 1)
        str << ", len:" << tkn.column_length;
    str << " :: '" << tkn.string_view() << "'";

    return str;
}

struct tokenizer {
    const char* position = nullptr;
    const char* const end = nullptr;
    uint current_char_length = 0;

    uint column = 0; // Optimization(sorgre): Only needed for setting column data inside tokens.

    static tokenizer from_string(const std::string &str) {
        return {str.data(), str.data() + str.length(), 1, 1};
    }

    bool not_at_end() {
        assert(position <= end);
        return position < end;
    }
    
    char current_char(int offset = 0) {
        assert(position + offset < end);
        return position[offset];
    }
    
    void consume_current() {
        assert(position + current_char_length <= end);
        position += current_char_length;
        column++;
        current_char_length = 1;
    }
    
    bool consume_numeric() {
        assert(position < end);
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

    template <int N>
    bool consume_good_utf8(const uint32_t (&unicode_table)[N][2]) {
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

        if (!(current_char() & UTF8_MULTIBYTE_MASK))
            return false;

        uint32_t code = 0;
        if ((current_char() & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
            if (end - position < 4) {
                std::cerr << "UTF8 string error: missing trailing bytes" << std::endl;
            }
            uint32_t b0 = (current_char(0) & UTF8_4BYTE_MASK_INV) << 18;
            uint32_t b1 = (current_char(1) & UTF8_NEXT_BYTE_MASK) << 12;
            uint32_t b2 = (current_char(2) & UTF8_NEXT_BYTE_MASK) << 6;
            uint32_t b3 = (current_char(3) & UTF8_NEXT_BYTE_MASK) << 0;
            code = b0 | b1 | b2 | b3;
            current_char_length = 4;
        }
        else if ((current_char() & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
            if (end - position < 3) {
                std::cerr << "UTF8 string error: missing trailing bytes" << std::endl;
            }
            uint32_t b0 = (current_char(0) & UTF8_3BYTE_MASK_INV) << 12;
            uint32_t b1 = (current_char(1) & UTF8_NEXT_BYTE_MASK) << 6;
            uint32_t b2 = (current_char(2) & UTF8_NEXT_BYTE_MASK) << 0;
            code = b0 | b1 | b2;
            current_char_length = 3;
        }
        else if ((current_char() & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
            if (end - position < 2) {
                std::cerr << "UTF8 string error: missing trailing bytes" << std::endl;
            }
            uint32_t b0 = (current_char(0) & UTF8_2BYTE_MASK_INV) << 6;
            uint32_t b1 = (current_char(1) & UTF8_NEXT_BYTE_MASK) << 0;
            code = b0 | b1;
            current_char_length = 2;
        }
        else {
            std::cerr << "UTF8 string error: bad code point '" << current_char() << "'" << std::endl;
        }
        
        for (size_t i = 0; i < N; i++) {
            if (code >= unicode_table[i][0] && code <= unicode_table[i][1]) {
                consume_current();
                return true;
            }
        }

        return false;
    }

    token next_token() {
        while (not_at_end()) {

            token current_token = {
                token::bad_char_kind,
                position, 1,
                column, 1
            };
    
            // Single (byte) punctuation characters
            switch (current_char()) {
                case ' ':
                case '\t':
                    consume_current();
                    continue;
                case '(':
                case ')':
                case '+':
                case '-':
                case '*':
                case '/':
                case '=':
                    current_token.kind = token::punctuation_kind;
                    consume_current();
                    return current_token;
            }

            // Number literal
            while (consume_numeric() && not_at_end());
            // Number literal, decimal/fractional part
            if (not_at_end() && current_char() == '.') {
                consume_current();
                while (not_at_end() && consume_numeric());
            }

            if (current_token.position != position) {
                // Number literal has been parsed
                current_token.kind = token::number_kind;
                current_token.length = position - current_token.position;
                current_token.column_length = column - current_token.column;
                return current_token;
            }

            // identifier
            current_char_length = 1;
            if (
                not_at_end() && (
                    consume_alpha() ||
                    consume_good_utf8(id_start_table)
                )
            )
                while (
                    not_at_end() && (
                        consume_alpha() ||
                        consume_numeric() ||
                        consume_good_utf8(id_continue_table)
                    )
                );

            if (current_token.position != position) {
                // Identifier has been parsed
                current_token.kind = token::symbol_kind;
                current_token.length = position - current_token.position;
                current_token.column_length = column - current_token.column;
                return current_token;
            }

            current_token.kind = token::bad_char_kind;
            current_token.length = current_char_length;
            std::cout << current_token << std::endl;
            consume_current();
            return current_token;
        };

        return {
                token::eol_kind,
                position, 1,
                column, 1
        };
    }
};