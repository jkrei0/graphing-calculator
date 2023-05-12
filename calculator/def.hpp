#pragma once
#include <vector>
#include <string>
#include <iostream>

enum class o {
    none,
    add,
    subtract,
    negate,
    multiply,
    divide,
    exponent,
    function,
    modulo
};
enum class t {
    none,
    group,
    number,
    operation,
    variable
};
struct Token {
    t type{ t::none };
    std::vector<Token> group{ };
    double number{ 0 };
    o operation{ o::none };
    std::string function{ };
    char variable{ '&' };
};
using TokenArr = std::vector<Token>;

struct Grid {
    double startX{ -12 };
    double startY{ -8 };
    double endX{ 12 };
    double endY{ 8 };
    double stepX{ 0.25 }; // half b/c cmd characters are ~ half as wide as tall
    double stepY{ 0.5 };
    std::vector<std::vector<double>> points{ };
};

// Unary (o::negate) operations
// use the RIGHT side as their operand
struct TreeItem {
    bool solved{ false };
    bool isVariable{ false };
    double value{ 0 };
    char variable{ '&' };
    o operation{ o::none };
    std::string function{ };
    TreeItem* left{ nullptr };
    TreeItem* right{ nullptr };
};

std::string tAsString(t name) {
    switch(name) {
        case t::none: return "none";
        case t::group: return "group";
        case t::number: return "number";
        case t::operation: return "operation";
        case t::variable: return "variable";
        default: return "t?";
    }
}
std::string oAsString(o name) {
    switch(name) {
        case o::none: return "none";
        case o::add: return "add";
        case o::subtract: return "subtract";
        case o::negate: return "negate";
        case o::modulo: return "modulo";
        case o::multiply: return "multiply";
        case o::divide: return "divide";
        case o::exponent: return "exponent";
        case o::function: return "function";
        default: return "o?";
    }
}
void printToken(Token tk) {
    std::cout << tAsString(tk.type) << "= " << oAsString(tk.operation) << ", " << tk.number << ", " << tk.variable << ", f" << tk.function << "\n";
    if (tk.type == t::group) {
        for (const Token& child : tk.group) {
           std::cout << "    ";
           printToken(child);
        }
        std::cout << "<-\n";
    }
}

std::string getLine(std::string prompt) {
    std::string input{ };
    // Prompt for input
    std::cout << prompt;
    std::getline(std::cin >> std::ws, input); // ws to ignore leading whitespace.
    return input;
}
// Clears anything waiting in the input buffer
void clearCin() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
// Has the user enter a number
double getNumber(std::string prompt) {
    double input{ };
    while (true) {
        // Prompt for input
        std::cout << prompt;
        std::cin >> input;
        
        // If failure, clear it and try again
        if (std::cin.fail()) {
            std::cin.clear();
            clearCin();

        } // Else break and return the value
        else { 
            clearCin();
            return input;
        }
        std::cout << "That's not valid. ";
    }
}
