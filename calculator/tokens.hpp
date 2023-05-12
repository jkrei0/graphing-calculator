#pragma once
#include <regex>
#include "def.hpp"

TokenArr tokenize(std::string equation, bool& error) {
    TokenArr out{ };
    const std::regex rGroup{ "^\\(" };
    const std::regex rNumber{ "^(\\d+(\\.\\d+)?|\\.\\d+)" };
    const std::regex rOperator{ "^(\\+|-|\\*|\\/|\\^|%)" };
    const std::regex rVariable{ "^[a-z]" };
    const std::regex rFunction{ "^[A-Z]+" };
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
                case '%': newTk.operation = o::modulo; break;
                default:
                    std::cout << "<!> [tokenize:0] Unknown operator " << match.str() << "\n";
                    error = true;
            }
            out.push_back(newTk);

        } else if (std::regex_search(equation, match, rVariable)) {
            Token newTk{ t::variable };
            newTk.variable = match.str().at(0);
            out.push_back(newTk);
            
        } else if (std::regex_search(equation, match, rFunction)) {
            Token newTk{ t::operation };
            newTk.function = match.str();
            newTk.operation = o::function;
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
        try {
            equation = equation.substr(match.length(), equation.size());
        } catch (std::out_of_range const&) {
            std::cout << "<!> [tokenize:3] Abrupt end of equation! " << equation << '\n';
            error = true;
            return out;
        }
    }

    return out;
}
/** Prepares a TokenArr for parsing. OPERATES DIRECTLY ON THE PASSED LIST
 * - replaces o::subtract with o::negate or o::add o::negate
 */
bool clean(TokenArr& list) {
    if (list.size() < 1) {
        std::cout << "(I) [clean:2] Equation or group is zero-length, adding implicit 0.\n";
        Token newTk{ t::number };
        newTk.number = 0;
        list.push_back(newTk);
        return true;
    }

    // Special case if the list starts with o::subtract
    if (list.at(0).operation == o::subtract) {
        list.at(0).operation = o::negate;
    }
    if (list.at(list.size()-1).type == t::operation) {
        std::cout << "<!> [clean:0] Equation contains a trailing operator\n";
        return false;
    }

    for (std::size_t i{ 0 }; i < list.size(); i++) {
        if (list.at(i).type == t::group) {
            // Recursively clean groups
            // Fail if groups fails
            if (!clean(list.at(i).group)) return false;
        }

        // The following checks assume they start on the second element,
        // So we skip the first (0th) element now.
        if (i == 0) continue;

        // --> functions are unary, so, eg 8SINx => 8*SINx
        if ((list.at(i).type != t::operation || list.at(i).operation == o::function) && list.at(i-1).type != t::operation) {
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

        } else if (list.at(i).operation == o::add && list.at(i-1).operation == o::subtract) {
            std::cout << "<!> [clean:3] Add operator following subtraction operator. You probably meant a+-b" << i << '\n';
            return false;
        }
    }
    return true;
}
