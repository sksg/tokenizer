#include <iostream>
#include <string>

int main() {
    std::cout << "Welcome to the tokenizer. " << std::endl;
    std::cout << "Input a line of code, and the tokenizer will return the tokens. " << std::endl;
    std::cout << "Exit by closing input stream e.g. ctrl+d (unix) or ctrl+z (win)." << std::endl;
    std::cout << std::endl;

    while (true) { //REPL lopp
        std::string next_line;

        // Tokenizer prompt:
        std::cout << "tokenizer> " << std::flush;

        if (!std::getline(std::cin, next_line)) {
            std::cout << "exiting..." << std::endl;
            break;
        }

        
        std::cout << next_line << std::endl;
    }
    return EXIT_SUCCESS;
}
