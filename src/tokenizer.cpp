#include "tokenizer.hpp"

template <int N>
bool run_test(const std::string &input, const token (&output)[N]) {
    auto tokenizer = tokenizer::from_string(input);
    for (auto out : output) {
        auto next_token = tokenizer.next_token();
        if (
            (next_token.kind != out.kind) ||
            (next_token.position != out.position) ||
            (next_token.length != out.length) ||
            (next_token.column != out.column) ||
            (next_token.column_length != out.column_length)
        ) {
            std::cerr << "Expected token: " << out << std::endl;
            std::cerr << "got token: " << next_token << std::endl;
            return false;
        }
    }
    if (tokenizer.position != tokenizer.end) {
        std::cerr << "Expected tokenizer at end, but it is not" << std::endl;
        return false;
    }
    return true;
}

void run_all_tests() {
    {
        std::string input = "test";
        assert(
            run_test(
                input,
                {
                    {token::symbol_kind, input.data(), input.length(), 1, input.length()},
                    {token::eol_kind, input.data() + input.length(), 1, 1 + input.length(), 1}
                }
            )
        );
    }
    {
        std::string input = "test1234";
        assert(
            run_test(
                input,
                {
                    {token::symbol_kind, input.data(), input.length(), 1, input.length()},
                    {token::eol_kind, input.data() + input.length(), 1, 1 + input.length(), 1}
                }
            )
        );
    }
    {
        std::string input = "1234test";
        assert(
            run_test(
                input,
                {
                    {token::number_kind, input.data(), 4, 1, 4},
                    {token::symbol_kind, input.data() + 4, 4, 5, 4},
                    {token::eol_kind, input.data() + input.length(), 1, 1 + input.length(), 1}
                }
            )
        );
    }
    {
        std::string input = "1234test(µ)";
        assert(
            run_test(
                input,
                {
                    {token::number_kind, input.data(), 4, 1, 4},
                    {token::symbol_kind, input.data() + 4, 4, 5, 4},
                    {token::punctuation_kind, input.data() + 8, 1, 9, 1},
                    {token::symbol_kind, input.data() + 9, 2, 10, 1},
                    {token::punctuation_kind, input.data() + 11, 1, 11, 1},
                    {token::eol_kind, input.data() + input.length(), 1, input.length(), 1}
                }
            )
        );
    }
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
        auto tokenizer = tokenizer::from_string(line);
        while (true) {
            auto next_token = tokenizer.next_token();
            std::cout << next_token << std::endl;

            if (next_token.kind != token::eol_kind)
                break;
        };
    }

    std::cout << "Exiting REPL..." << std::endl;

    run_all_tests();

    return EXIT_SUCCESS;
}

