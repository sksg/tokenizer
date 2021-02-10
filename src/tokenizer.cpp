#include <iostream>
#include <string>
#include <sstream>

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
                    if (identifier_start < position) {
                        size_t length = position - identifier_start;
                        std::cout << next_line.substr(identifier_start, length) << std::endl;
                    }
                    identifier_start = position + 1;
                    continue;
                case '(':
                case ')':
                case '[':
                case ']':
                case '{':
                case '}':
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
                case '#':
                case '@':
                case '"':
                case '\'':
                    if (identifier_start < position) {
                        size_t length = position - identifier_start;
                        std::cout << next_line.substr(identifier_start, length) << std::endl;
                    }
                    identifier_start = position + 1;
                    std::cout << next_char << std::endl;
                    continue;
            }
        }
        if (identifier_start < position) {
            size_t length = position - identifier_start;
            std::cout << next_line.substr(identifier_start, length) << std::endl;
        }
        std::cout << "EOL" << std::endl;
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

