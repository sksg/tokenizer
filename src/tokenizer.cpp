#include "tokenizer.hpp"

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
        auto tokenizer = tokenizer::from_string(line);
        token next_token;
        do {
            next_token = tokenizer.next_token();
            std::cout << next_token << std::endl;
        }
        while (next_token.kind != token::eol_kind);
    }

    std::cout << "Exiting REPL..." << std::endl;
    return EXIT_SUCCESS;
}

