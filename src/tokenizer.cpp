#include <iostream>
#include <string>
#include <sstream>

// Test strings:
// a = 1
// b=2
// c =3
// a+b * c

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
        size_t identifier_start = 0;
        size_t position = 0;
        for (; position < next_line.length(); position++) {
            auto next_char = next_line[position];
    
            switch (next_char) {
                case ' ':
                case '\t':
                    continue; // gooble, gooble
                case '(':
                case ')':
                case '[':
                case ']':
                case '<':
                case '>':
                case '+':
                case '-':
                case '*':
                case '/':
                case '&':
                case '|':
                case '!':
                case '=':
                case '"':
                    std::cout << "OPR @ col:" << position + 1;
                    std::cout << " :: '" << next_char << "'" << std::endl;
                    continue;
            }

            identifier_start = position;

            if (next_char >= '0' && next_char <= '9') {
                // Number literal
                do {
                    position++;
                    next_char = next_line[position];
                } while (next_char >= '0' && next_char <= '9');
            }
            if (next_char == '.') {
                // Number literal, decimal/fractional part
                do {
                    position++;
                    next_char = next_line[position];
                } while (next_char >= '0' && next_char <= '9');
            }
            
            if (identifier_start != position) {
                auto length = position - identifier_start;
                auto number = next_line.substr(identifier_start, length);
                std::cout << "NUM @ col:" << identifier_start + 1 << ", len:" << length;
                std::cout << " :: '" << number << "'" << std::endl;
                position--;
                continue;
            }

            if (
                (next_char >= 'a' && next_char <= 'z') ||
                (next_char >= 'A' && next_char <= 'Z')
            ) {
                // Number literal
                do {
                    position++;
                    next_char = next_line[position];
                } while (
                    (next_char >= 'a' && next_char <= 'z') ||
                    (next_char >= 'A' && next_char <= 'Z') ||
                    (next_char >= '0' && next_char <= '9') ||
                    (next_char == '_')
                );
            }
            
            if (identifier_start != position) {
                auto length = position - identifier_start;
                auto symbol = next_line.substr(identifier_start, length);
                std::cout << "SYM @ col:" << identifier_start + 1 << ", len:" << length;
                std::cout << " :: '" << symbol << "'" << std::endl;
                position--;
            }
        }
        std::cout << "EOL" << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

