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
        std::cout << next_line << std::endl;
    }

    std::cout << "input closed, exiting..." << std::endl;
    return EXIT_SUCCESS;
}
