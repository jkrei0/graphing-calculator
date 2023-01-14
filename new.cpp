#include <iostream>
#include <string>
#include <vector>
#include <regex>

std::string getLine(std::string prompt) {
    std::string input{ };
    // Prompt for input
    std::cout << prompt;
    std::getline(std::cin >> std::ws, input); // ws to ignore leading whitespace.
    return input;
}

enum class o {
    none,
    add,
    subtract,
    negate,
    multiply,
    divide,
    exponent
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
    char variable{ '&' };
};
using TokenArr = std::vector<Token>;

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
        case o::multiply: return "multiply";
        case o::divide: return "divide";
        case o::exponent: return "exponent";
        default: return "o?";
    }
}
void printToken(Token tk) {
    std::cout << tAsString(tk.type) << "= " << oAsString(tk.operation) << ", " << tk.number << ", " << tk.variable << "\n";
    if (tk.type == t::group) {
        for (const Token& child : tk.group) {
           std::cout << "\t";
           printToken(child);
        }
        std::cout << "<-\n";
    }
}

TokenArr tokenize(std::string equation, bool& error) {
    TokenArr out{ };
    const std::regex rGroup{ "^\\(" };
    const std::regex rNumber{ "^\\d+(\\.\\d+)?|\\.\\d+" };
    const std::regex rOperator{ "^(\\+|-|\\*|\\/|\\^)" };
    const std::regex rVariable{ "^[a-z]" };
    const std::regex rWhitespace{ "^ +" };
    std::smatch match;

    while (true) {
        if (std::regex_search(equation, match, rNumber)) {
            Token newTk{ t::number };
            newTk.number = std::stod(match.str());
            out.push_back(newTk);

        } else if (std::regex_search(equation, match, rOperator)) {
            Token newTk{ t::operation };
            switch (match.str()[0]) {
                case '+': newTk.operation = o::add;      break;
                case '-': newTk.operation = o::subtract; break;
                case '*': newTk.operation = o::multiply; break;
                case '/': newTk.operation = o::divide;   break;
                case '^': newTk.operation = o::exponent; break;
                default:
                    std::cout << "<!> [tokenize:0] Unknown operator " << match.str() << "\n";
                    error = true;
            }
            out.push_back(newTk);

        } else if (std::regex_search(equation, match, rVariable)) {
            Token newTk{ t::variable };
            newTk.variable = match.str().at(0);
            out.push_back(newTk);
            
        } else if (std::regex_search(equation, match, rGroup)) {
            Token newTk{ t::group };

            std::size_t pos{ 1 };
            std::size_t depth{ 0 };
            while (pos < equation.size()) {
                if (equation.at(pos) == ')') {
                    if (depth == 0) break;
                    else depth -= 1;
                
                } else if (equation.at(pos) == '(') {
                    depth += 1;
                }
                pos++;
            }
            if (depth > 0) {
                std::cout << "<!> [tokenize:1] Unmatched parentheses: " << equation << '\n';
                error = true;
                break;
            }
            newTk.group = tokenize(equation.substr(1, pos-1), error);
            out.push_back(newTk);
            equation = equation.substr(pos, equation.size());
            
        } else if (!std::regex_search(equation, match, rWhitespace)) {
            // No match and not whitespace
            if (equation.size() > 0) {
                // It's only a problem if we're not done already.
                std::cout << "<!> [tokenize:2] Cannot parse part of equation: " << equation << '\n';
                error = true;
            }
            break;
        }

        equation = equation.substr(match.length(), equation.size());
    }

    return out;
}
/** Prepares a TokenArr for parsing. OPERATES DIRECTLY ON THE PASSED LIST
 * - replaces o::subtract with o::negate or o::add o::negate
 */
bool clean(TokenArr& list) {
    if (list.at(0).operation == o::subtract) {
        list.at(0).operation = o::negate;
    }
    if (list.at(list.size()-1).type == t::operation) {
        std::cout << "<!> [clean:0] Equation contains a trailing operator\n";
        return false;
    }
    // First already checked, so loop from 1
    for (std::size_t i{ 1 }; i < list.size(); i++) {
        if (list.at(i).type != t::operation && list.at(i-1).type != t::operation) {
            // values next to each other are implicitly multiplied
            // insert an o::multiply operator
            Token newTk{ t::operation };
            newTk.operation = o::multiply;
            list.insert(list.begin()+i, newTk);
        }

        // Anything that works on operators assumes no trailing operators
        // and might try to access the next element.
        if (i >= list.size()-1) break;

        if (list.at(i).type == t::operation &&
            list.at(i-1).type == t::operation &&
            list.at(i+1).type == t::operation)
        {
            std::cout << "<!> [clean:1] Equation contains too many successive operators (3+) at position " << i << '\n';
            return false;
        }

        if (list.at(i).operation == o::subtract && list.at(i+1).operation == o::subtract) {
            // Replace double negatives with addition
            list.at(i).operation = o::add;
            list.erase(list.begin()+i+1);

        } else if (list.at(i).operation == o::subtract && list.at(i-1).type == t::operation) {
            // Eg 3+(subtract)4 => 3+(negate)4
            list.at(i).operation = o::negate;

        } else if (list.at(i).operation == o::subtract) {
            // Previous now must be a value
            // Eg 3(subtract)4 => 3+(negate)4
            // Edge case 3-+4 => 3+-+ 4 => Error on next check & I don't
            // have to worry about it here!
            list.at(i).operation = o::add;
            // insert o::negate after o::add
            Token newTk{ t::operation };
            newTk.operation = o::negate;
            list.insert(list.begin()+i+1, newTk);

        } else if (list.at(i).type == t::group) {
            // Recursively clean groups
            // Fail if groups fails
            if (!clean(list.at(i).group)) return false;
        }
    }
    return true;
}

int main() {
    std::cout << "GRAPHING CALCULATOR v2\n";
    std::string equation{ getLine("> ") };
    bool error{ false };
    TokenArr tokenized{ tokenize(equation, error) };
    if (error || !clean(tokenized)) {
        std::cout << "<!> [main:0] Parsing failed\n";
    }
    for (const Token& tk : tokenized) {
        printToken(tk);
    }

    // std::regex regex{ "\\(" };
    // std::smatch match;
    // std::regex_search(equation, match, regex);
    // std::cout << "Match: " << match.str() << "\n";

    std::cout << "\nDone!\n";
    return 0;
}
