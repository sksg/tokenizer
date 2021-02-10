#include <iostream>
#include <string>

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
        for (size_t i = 0; i < next_line.length();) {
            int code_point_len = 1;
            if ((next_line[i] & 0xf8) == 0xf0)             code_point_len = 4;
            else if ((next_line[i] & 0xf0) == 0xe0)        code_point_len = 3;
            else if ((next_line[i] & 0xe0) == 0xc0)        code_point_len = 2;
            if ((i + code_point_len) > next_line.length()) code_point_len = 1;

            std::string utf8_char = next_line.substr(i, code_point_len);
            std::cout << utf8_char << " [" << code_point_len << "]" << std::endl;

            i += code_point_len;
        }
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

