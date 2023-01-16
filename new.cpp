#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <stdexcept>
#include <map>
#include <cmath>
#include <limits>
#include <iomanip>

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

struct Grid {
    int startX{ -10 };
    int startY{ -10 };
    int endX{ 10 };
    int endY{ 10 };
    double stepX{ 0.5 }; // half b/c cmd characters are ~ half as wide as tall
    double stepY{ 1 };
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
           std::cout << "    ";
           printToken(child);
        }
        std::cout << "<-\n";
    }
}

TokenArr tokenize(std::string equation, bool& error) {
    TokenArr out{ };
    const std::regex rGroup{ "^\\(" };
    const std::regex rNumber{ "^(\\d+(\\.\\d+)?|\\.\\d+)" };
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

        }
    }
    return true;
}

TreeItem buildTree(const TokenArr& list) {
    TreeItem root{ };
    if (list.size() < 1) {
        throw std::invalid_argument("Cannot create a tree from a zero-length TokenArr");
    }

    std::vector<o> orderedOperations{ 
        o::add,
        o::multiply,
        o::divide,
        o::exponent,
        o::negate
    };
    for (const o currentOperation : orderedOperations) {
        for (std::size_t i{ 0 }; i < list.size(); i++) {
            if (list.at(i).type == t::operation && list.at(i).operation == currentOperation) {
                root.operation = currentOperation;
                TokenArr left{ };
                TokenArr right{ };
                // copy both sides into new vectors
                std::copy(list.begin(), list.begin() + i, std::back_inserter(left));
                std::copy(list.begin() + i+1, list.end(), std::back_inserter(right)); // +1 to not include the operation
                if (left.size() > 0) { // unary operators don't need a left
                    root.left = new TreeItem{ buildTree(left) };
                }
                if (right.size() < 1) throw ("Operator has no right operand!");
                root.right = new TreeItem{ buildTree(right) };

                return root;
            }
        }
    }
    if (list.at(0).type == t::group) {
        // Just a group? return the group.
        return buildTree(list.at(0).group);
    } else if (list.at(0).type == t::number) {
        root.solved = true;
        root.value = list.at(0).number;
    } else if (list.at(0).type == t::variable) {
        // Variables aren't marked as solved, only items
        // with a resolved numerical value is considered solved.
        root.isVariable = true;
        root.variable = list.at(0).variable;
    } else {
        throw ("Cannot create TreeItem from given TokenArr. No valid operators or values.");
    }

    if (list.size() > 1) {
        std::cout << "<?> [buildTree:0] List contains multiple values with no operation!\n";
        for (const Token& tk : list) {
            printToken(tk);
        }
    }
    return root;
}

void printTree(const TreeItem& item, int indentation = 0, std::string pos = "0") {
    std::string indent( indentation*4, ' ' );
    std::string indentNext( (indentation+1)*4, ' ' );
    std::cout << indent << pos << " = "
              << "operation: " << oAsString(item.operation)
              << "  value: " << item.value
              << "  variable: " << item.variable
              << "  isVariable: " << item.isVariable
              << "  solved: " << item.solved
              << "\n";
    if (item.left != nullptr) {
        printTree(*item.left, indentation + 1, "L");
    }
    if (item.right != nullptr) {
        printTree(*item.right, indentation + 1, "R");
    }
}

double solveOperation(o oper, double left, double right) {
    switch(oper) {
        case o::add: return left + right;
        case o::subtract: std::cout << "HOW DID A SUBTRACT GET IN HERE\n"; return left - right;
        case o::negate: return -right;
        case o::multiply: return left * right;
        case o::divide: return left / right;
        case o::exponent: return std::pow(left, right);
        case o::none:
        default: std::cout << "<?> [solveOperation:0] Unknown operation " << oAsString(oper) << ".\n"; return right;
    }
}
double solveTree(const TreeItem& item, std::map<char, double> variables) {
    double left{ 0 };
    double right{ 0 };
    for (const int i : {0, 1}) {
        double* v{ &left };
        TreeItem* side{ item.left };
        if (i == 1) {
            v = &right;
            side = item.right;
        }
        if (side != nullptr) {
            if (side->solved) {
                *v = side->value;
            } else if (side->isVariable && variables.contains(side->variable)) {
                *v = variables.find(side->variable)->second;
            } else if (side->operation != o::none) {
                *v = solveTree(*side, variables);
            }
        }
    }
    return solveOperation(item.operation, left, right);
}

Grid createGrid(const TreeItem& equation, const Grid& settings) {
    Grid out{ settings };

    if (out.startX >= out.endX || out.startY >= out.endY) {
        throw std::invalid_argument("Grid start positions must be less than end positions");
    }

    int xSteps{ (out.endX - out.startX)/out.stepX + 1 }; // +1 b/c grid ranges are inclusive
    int ySteps{ (out.endY - out.startY)/out.stepY + 1 };

    // clear any points that might've been copied from the settings
    out.points = { };
    out.points.resize(xSteps);

    for (int x{ 0 }; x < xSteps; x++) {
        std::vector<double>& xVector{ out.points.at(x) };
        xVector.resize(ySteps);

        for (int y{ 0 }; y < ySteps; y++) {
            // Solve for each point
            xVector.at(y) = solveTree(equation, {
                {'x', (x*out.stepX) + out.startX},
                {'y', (y*out.stepY) + out.startY}
            });
        }
    }

    return out;
}

void printGrid(const Grid& grid) {
    for (const std::vector<double>& xV : grid.points) {
        for (const double val : xV) {
            std::cout << val << ",\t";
        }
        std::cout << ";\n";
    }
}

// Draws a grid.
// IMPORTANT: The grid's settings must actually be the dimensions of the vectors!
void drawGrid(const Grid& grid) {
    double xSteps{ (grid.endX - grid.startX)/grid.stepX + 1 };
    double ySteps{ (grid.endY - grid.startY)/grid.stepY + 1 };
    std::cout << "=====\n";
    // y goes backwards so it's printed right-side-up
    for (int y{ ySteps-2 }; y > 0; y--) {
        std::cout << std::setw(5) << y*grid.stepY + grid.startY << ": ";
        for (int x{ 1 }; x < xSteps-2; x++) {
            double actualX{ x*grid.stepX + grid.startX };
            double actualY{ y*grid.stepY + grid.startY };

            bool sign{ grid.points.at(x).at(y) >= 0 };
            bool sT{ grid.points.at(x).at(y+1) >= 0 != sign };
            bool sB{ grid.points.at(x).at(y-1) >= 0 != sign };
            bool sR{ grid.points.at(x+1).at(y) >= 0 != sign };
            bool sL{ grid.points.at(x-1).at(y) >= 0 != sign };

            if (sT + sB + sR + sL > 1) std::cout << '8';
            else if (sT + sB + sR + sL > 0) std::cout << '*';
            else if (actualX == 0) std::cout << '|';
            else if (actualY == 0) std::cout << '-';
            else std::cout << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "=====\n";
}

int main() {
    bool verbose{ false };
    std::cout << "GRAPHING CALCULATOR v2\n";

    while (true) {
        std::string equation{ getLine("Enter an equation: 0 = ") };

        bool error{ false };
        std::cout << "(I) Tokenizing equation...\n";
        TokenArr tokenized{ tokenize(equation, error) };

        std::cout << "(I) Cleaning equation...\n";
        if (error || !clean(tokenized)) {
            std::cout << "<!> [main:0] Parsing failed\n";
            for (const Token& tk : tokenized) {
                printToken(tk);
            }
            continue;
        }

        if (verbose) {
            for (const Token& tk : tokenized) {
                printToken(tk);
            }
        }

        std::cout << "(I) Creating tree...\n";
        TreeItem tree{ buildTree(tokenized) };

        if (verbose) printTree(tree);

        Grid g{ createGrid(tree, { }) };
        if (verbose) printGrid(g);
        drawGrid(g);

        // double xValue{ getNumber("Solve for x value: ") };

        // std::cout << "= " << solveTree(tree, {{'x', xValue}}) << "\n";

    }

    // std::regex regex{ "\\(" };
    // std::smatch match;
    // std::regex_search(equation, match, regex);
    // std::cout << "Match: " << match.str() << "\n";

    std::cout << "\nDone!\n";
    return 0;
}
