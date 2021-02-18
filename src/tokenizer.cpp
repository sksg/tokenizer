#include <iostream>
#include <string>
#include <sstream>
#include <cstdint>

#include "unicode_tables.h"

// Test strings:
// a = 1
// b=2
// c =3
// a+b * c

// multibyte test: 0b1xxxxxxx;
const uint8_t UTF8_MULTIBYTE_MASK = 0x80;  // 0b10000000;

// 4-byte test: 0b11110xxx;
const uint8_t UTF8_4BYTE_MASK = 0xf8;  // 0b11111000;
const uint8_t UTF8_4BYTE_MASK_INV = 0x07;  // 0b00000111;
const uint8_t UTF8_4BYTE_TEST = 0xf0;  // 0b11110000;

// 3-byte test: 0b1110xxxx;
const uint8_t UTF8_3BYTE_MASK = 0xf0;  // 0b11110000;
const uint8_t UTF8_3BYTE_MASK_INV = 0x0f;  // 0b00001111;
const uint8_t UTF8_3BYTE_TEST = 0xe0;  // 0b11100000;

// 2-byte test: 0b110xxxxx;
const uint8_t UTF8_2BYTE_MASK = 0xe0;  // 0b11100000;
const uint8_t UTF8_2BYTE_MASK_INV = 0x1f;  // 0b00001111;
const uint8_t UTF8_2BYTE_TEST = 0xc0;  // 0b11000000;

const uint8_t UTF8_NEXT_BYTE_MASK = 0x3f;  // 0b00111111;

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
            else if (current_char & UTF8_MULTIBYTE_MASK) {
                uint32_t code = 0;
                if ((current_char & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
                    if (position + 4 > next_line.length()) {
                        std::cerr << "UTF8 string error (missing bytes)" << std::endl;
                    }
                    uint32_t b0 = (next_line[position + 0] & UTF8_4BYTE_MASK_INV) << 18;
                    uint32_t b1 = (next_line[position + 1] & UTF8_NEXT_BYTE_MASK) << 12;
                    uint32_t b2 = (next_line[position + 2] & UTF8_NEXT_BYTE_MASK) << 6;
                    uint32_t b3 = (next_line[position + 3] & UTF8_NEXT_BYTE_MASK) << 0;
                    code = b0 | b1 | b2 | b3;
                    position += 4;
                }
                else if ((current_char & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
                    if (position + 3 > next_line.length()) {
                        std::cerr << "UTF8 string error (missing bytes)" << std::endl;
                    }
                    uint32_t b0 = (next_line[position + 0] & UTF8_3BYTE_MASK_INV) << 12;
                    uint32_t b1 = (next_line[position + 1] & UTF8_NEXT_BYTE_MASK) << 6;
                    uint32_t b2 = (next_line[position + 2] & UTF8_NEXT_BYTE_MASK) << 0;
                    code = b0 | b1 | b2;
                    position += 3;
                }
                else if ((current_char & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
                    if (position + 2 > next_line.length()) {
                        std::cerr << "UTF8 string error (missing bytes)" << std::endl;
                    }
                    uint32_t b0 = (next_line[position + 0] & UTF8_2BYTE_MASK_INV) << 6;
                    uint32_t b1 = (next_line[position + 1] & UTF8_NEXT_BYTE_MASK) << 0;
                    code = b0 | b1;
                    position += 2;
                }
                else {
                    std::cerr << "Unknown unicode character '" << current_char << "'";
                    std::cerr << "(\\U" << (uint)(uint8_t)current_char << ")" << std::endl;
                }
                
                for (auto range : id_start_table)
                    is_id_start |= code >= range[0] && code <= range[1];
            }

            if (is_id_start) {
                // identifier
                bool is_id_continue = true;
                while (is_id_continue && position < next_line.length()) {
                    current_char = next_line[position];
                    is_id_continue = (
                        (current_char >= 'a' && current_char <= 'z') ||
                        (current_char >= 'A' && current_char <= 'Z') ||
                        (current_char >= '0' && current_char <= '9') ||
                        (current_char == '_')
                    );
                    if (is_id_continue)
                        position++;
                    else if (current_char & UTF8_MULTIBYTE_MASK) {
                        uint32_t code = 0;
                        if ((current_char & UTF8_4BYTE_MASK) == UTF8_4BYTE_TEST) {
                            if (position + 4 > next_line.length()) {
                                std::cerr << "UTF8 string error (missing bytes)" << std::endl;
                            }
                            uint32_t b0 = (next_line[position + 0] & UTF8_4BYTE_MASK_INV) << 18;
                            uint32_t b1 = (next_line[position + 1] & UTF8_NEXT_BYTE_MASK) << 12;
                            uint32_t b2 = (next_line[position + 2] & UTF8_NEXT_BYTE_MASK) << 6;
                            uint32_t b3 = (next_line[position + 3] & UTF8_NEXT_BYTE_MASK) << 0;
                            code = b0 | b1 | b2 | b3;
                            position += 4;
                        }
                        else if ((current_char & UTF8_3BYTE_MASK) == UTF8_3BYTE_TEST) {
                            if (position + 3 > next_line.length()) {
                                std::cerr << "UTF8 string error (missing bytes)" << std::endl;
                            }
                            uint32_t b0 = (next_line[position + 0] & UTF8_3BYTE_MASK_INV) << 12;
                            uint32_t b1 = (next_line[position + 1] & UTF8_NEXT_BYTE_MASK) << 6;
                            uint32_t b2 = (next_line[position + 2] & UTF8_NEXT_BYTE_MASK) << 0;
                            code = b0 | b1 | b2;
                            position += 3;
                        }
                        else if ((current_char & UTF8_2BYTE_MASK) == UTF8_2BYTE_TEST) {
                            if (position + 2 > next_line.length()) {
                                std::cerr << "UTF8 string error (missing bytes)" << std::endl;
                            }
                            uint32_t b0 = (next_line[position + 0] & UTF8_2BYTE_MASK_INV) << 6;
                            uint32_t b1 = (next_line[position + 1] & UTF8_NEXT_BYTE_MASK) << 0;
                            code = b0 | b1;
                            position += 2;
                        }
                        else {
                            std::cerr << "Unknown unicode character '" << current_char << "'";
                            std::cerr << "(\\U" << (uint)(uint8_t)current_char << ")" << std::endl;
                        }
                        
                        for (auto range : id_start_table)
                            is_id_continue |= code >= range[0] && code <= range[1];
                    }
                };
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

