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
#include <cmath>
#ifndef SNUMBERS
#include <numbers>
#endif
#ifdef SNUMBERS
// Older g++ doesn't have <numbers>
namespace std::numbers {
    double pi{ 3.1415926535 };
    double e{ 2.7182818284 };
}
#endif

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
    exponent,
    function
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
    int startX{ -12 };
    int startY{ -8 };
    int endX{ 12 };
    int endY{ 8 };
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

TokenArr tokenize(std::string equation, bool& error) {
    TokenArr out{ };
    const std::regex rGroup{ "^\\(" };
    const std::regex rNumber{ "^(\\d+(\\.\\d+)?|\\.\\d+)" };
    const std::regex rOperator{ "^(\\+|-|\\*|\\/|\\^)" };
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

TreeItem buildTree(const TokenArr& list) {
    TreeItem root{ };
    if (list.size() < 1) {
        throw std::invalid_argument("Cannot create a tree from a zero-length TokenArr");
    }

    std::vector<o> orderedOperations{ 
        o::add,
        o::subtract,
        o::multiply,
        o::divide,
        o::exponent,
        o::negate,
        o::function
    };
    for (const o currentOperation : orderedOperations) {
        for (std::size_t i{ 0 }; i < list.size(); i++) {
            if (list.at(i).type == t::operation && list.at(i).operation == currentOperation) {
                root.operation = currentOperation;
                root.function = list.at(i).function;
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
              << "  function: " << item.function
              << "\n";
    if (item.left != nullptr) {
        printTree(*item.left, indentation + 1, "L");
    }
    if (item.right != nullptr) {
        printTree(*item.right, indentation + 1, "R");
    }
}

double doFunction(std::string name, double value = 0) {
    static double _logbase{ 2 };
    static double _modbase{ 3 };
    // No NTHROOT function because you can do that with x^(1/y)
    // Trigonometry
    if      (name == "SIN")   return std::sin(value);
    else if (name == "ASIN")  return std::asin(value);
    else if (name == "COS")   return std::cos(value);
    else if (name == "ACOS")  return std::acos(value);
    else if (name == "TAN")   return std::tan(value);
    else if (name == "ATAN")  return std::atan(value);
    // Roots
    else if (name == "SQRT")  return std::sqrt(value);
    else if (name == "CBRT")  return std::pow(value, (1/3));
    // Logarithms
    else if (name == "SETN")  {
        _logbase = value; // Because functions can't take two arguments, 'n'
        return 0;         // must be set before using LOGN
    }
    else if (name == "LOG")   return std::log10(value);
    else if (name == "LOGE")  return std::log(value);
    else if (name == "LOGN")  return std::log(value) / std::log(_logbase);
    // Etc
    else if (name == "ABS" && value >= 0)  return value;
    else if (name == "ABS" && value < 0)   return -value;
    else if (name == "SIGN" && value > 0)  return 1;
    else if (name == "SIGN" && value == 0) return 0;
    else if (name == "SIGN" && value < 0)  return -1;
    // Modulo
    else if (name == "EVEN")  return std::fmod(value, 2);
    else if (name == "SETM")  {
        _modbase = value; // Because functions can't take two arguments, 'n'
        return 0;         // must be set before using LOGN
    }
    else if (name == "MOD")  return std::fmod(value, _modbase);
    // Math constants
    else if (name == "PI" && value != 0)  return std::numbers::pi * value;
    else if (name == "PI")    return std::numbers::pi;
    else if (name == "E" && value != 0)  return std::numbers::e * value;
    else if (name == "E")     return std::numbers::e;
    else {
        std::cout << "<?> Unknown function " << name << "\n";
        return 0;
    }
}

double solveOperation(o oper, double left, double right, std::string function = "") {
    switch(oper) {
        case o::add: return left + right;
        case o::subtract: return left - right;
        case o::negate: return -right;
        case o::multiply: return left * right;
        case o::divide: return left / right;
        case o::exponent: return std::pow(left, right);
        case o::function: return doFunction(function, right);
        case o::none:
        default: std::cout << "<?> [solveOperation:0] Unknown operation " << oAsString(oper) << ".\n"; return right;
    }
}
double solveTree(const TreeItem& item, std::map<char, double> variables) {
    double left{ 0 };
    double right{ 0 };
    // {0, 1} so right happens first and
    // Eg LOGN(SETN(4)+ x*LOGN(SETN(5)+ y))
    // would set N to 5 AFTER 4 and the original log would use 4.
    // If the left is evaluated last, LOGN(SETN(x)+ ... will always 
    // be log base x
    for (const int i : {1, 0}) {
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
    return solveOperation(item.operation, left, right, item.function);
}

Grid createGrid(const TreeItem& equation, const Grid& settings) {
    Grid out{ settings };

    if (out.startX >= out.endX || out.startY >= out.endY) {
        throw std::invalid_argument("Grid start positions must be less than end positions");
    }

    int xSteps{ (int)std::floor((out.endX - out.startX)/out.stepX + 1) }; // +1 b/c grid ranges are inclusive
    int ySteps{ (int)std::floor((out.endY - out.startY)/out.stepY + 1) };

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
    for (int y{ (int)ySteps-2 }; y > 0; y--) {
        std::cout << std::setw(6) << y*grid.stepY + grid.startY << ": ";
        for (int x{ 1 }; x < xSteps-2; x++) {
            double actualX{ x*grid.stepX + grid.startX };
            double actualY{ y*grid.stepY + grid.startY };

            bool sign{ grid.points.at(x).at(y) >= 0 };
            bool sT{ (grid.points.at(x).at(y+1) >= 0) != sign };
            bool sB{ (grid.points.at(x).at(y-1) >= 0) != sign };
            bool sR{ (grid.points.at(x+1).at(y) >= 0) != sign };
            bool sL{ (grid.points.at(x-1).at(y) >= 0) != sign };

            if (sT + sB + sR + sL > 1) std::cout << '8';
            else if (sT + sB + sR + sL > 0) std::cout << '*';
            else if (std::abs(actualX) < grid.stepX/2) std::cout << '|';
            else if (std::abs(actualY) < grid.stepY/2) std::cout << '-';
            else std::cout << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "=====\n";
}
// Menu commands. Returns true on :quit, false otherwise
bool menu(std::string name, Grid& grid, TreeItem& tree, std::string last) {
    struct Save{
        std::string name;
        TreeItem tree;
    };
    static std::vector<Save> savedEquations{ };

    if (name == ":window") {
        std::cout << "Current grid:\n"
                  << "    startX: " << grid.startX << "\n"
                  << "    startY: " << grid.startY << "\n"
                  << "    endX:   " << grid.endX   << "\n"
                  << "    endY:   " << grid.endY   << "\n"
                  << "    stepX:  " << grid.stepX  << "\n"
                  << "    stepY:  " << grid.stepY  << "\n"
                  << "--\n";

    } else if (name == ":wedit") {
        menu(":window", grid, tree, last);
        grid.startX = getNumber("Enter startX: ");
        grid.startY = getNumber("Enter startY: ");

        double endX{ getNumber("Enter endX: ") };
        while (grid.startX >= endX) {
            std::cout << "endX must be greater than startX!\n";
            endX = getNumber("Enter endX: ");
        }
        grid.endX = endX;

        double endY{ getNumber("Enter endY: ") };
        while (grid.startY >= endY) {
            std::cout << "endY must be greater than startY!\n";
            endY = getNumber("Enter endY: ");
        }
        grid.endY = endY;

        grid.stepX = getNumber("Enter stepX: ");
        grid.stepY = getNumber("Enter stepY: ");

    } else if (name == ":solve" || name == ":v") {
        if (tree.function == "0") {
            std::cout << "Enter an equation first, then type :solve\n";
            return false;
        }
        double xValue{ getNumber("x value: ") };
        double yValue{ getNumber("y value: ") };

        double result{ solveTree(tree, {{'x', xValue}, {'y', yValue}}) };

        std::cout << "Point (" << xValue << ", " << yValue << ") = " << result << "\n";

    } else if (name == ":save" || name == ":s") {
        if (tree.function == "0") {
            std::cout << "Enter an equation first, then type :save\n";
            return false;
        }
        savedEquations.push_back({ last, tree });

        std::cout << "Saved equation to slot #" << savedEquations.size()-1 << "\n";

    } else if (name == ":list" || name == ":ls") {
        if (savedEquations.size() == 0) {
            std::cout << "No saved equations.\n";
            return false;
        }
        for (std::size_t i{ 0 }; i < savedEquations.size(); i++) { 
            std::cout << "#" << i << ": " << savedEquations.at(i).name << "\n";
        }


    } else if (name == ":regraph") {
        std::cout << "Graphing... 0 = " << last << "\n";
        drawGrid(createGrid(tree, grid));

    } else if (name == ":recall" || name == ":rs") {
        menu(":list", grid, tree, last);
        if (savedEquations.size() == 0) { return false; } // Message already sent by :list
        std::size_t slot{ (std::size_t)getNumber("Enter slot #: ") };
        if (slot >= savedEquations.size()) {
            std::cout << "Slot is empty.\n";
            return false;
        }
        Save recalled{ savedEquations.at(slot) };

        menu(":regraph", grid, tree, last);

    } else if (name == ":zoomin" || name == ":i") {

        double two{2}; // b/c stupid int division
        double xDist{ (grid.endX - grid.startX) / two };
        double centerX{ xDist + grid.startX };
        double yDist{ (grid.endY - grid.startY) / two };
        double centerY{ yDist + grid.startY };

        grid.startX = centerX-(xDist/two);
        grid.endX = centerX+(xDist/two);
        grid.startY = centerY-(yDist/two);
        grid.endY = centerY+(yDist/two);
        grid.stepX *= 0.5;
        grid.stepY *= 0.5;

        menu(":regraph", grid, tree, last);

    } else if (name == ":zoomout" || name == ":o") {

        double two{2}; // b/c stupid int division
        double xDist{ (grid.endX - grid.startX) / two };
        double centerX{ xDist + grid.startX };
        double yDist{ (grid.endY - grid.startY) / two };
        double centerY{ yDist + grid.startY };

        grid.startX = centerX-(xDist*two);
        grid.endX = centerX+(xDist*two);
        grid.startY = centerY-(yDist*two);
        grid.endY = centerY+(yDist*two);
        grid.stepX *= two;
        grid.stepY *= two;

        menu(":regraph", grid, tree, last);

    } else if (name == ":center" || name == ":cg") {

        double two{2}; // b/c stupid int division
        double xDist{ (grid.endX - grid.startX) / two };
        double yDist{ (grid.endY - grid.startY) / two };

        grid.startX = -(xDist);
        grid.endX   =  (xDist);
        grid.startY = -(yDist);
        grid.endY   =  (yDist);

        menu(":regraph", grid, tree, last);

    } else if (name == ":r" || name == ":l" || name == ":u" || name == ":d") {

        double two{2}; // b/c stupid int division
        double xDist{ (grid.endX - grid.startX) / two };
        double yDist{ (grid.endY - grid.startY) / two };
        if (name == ":r") {
            grid.startX += xDist * 0.5;
            grid.endX += xDist * 0.5;
        } else if (name == ":l") {
            grid.startX -= xDist * 0.5;
            grid.endX -= xDist * 0.5;
        } else if (name == ":u") {
            grid.startY += yDist * 0.5;
            grid.endY += yDist * 0.5;
        } else { // (name == ":d")
            grid.startY -= yDist * 0.5;
            grid.endY -= yDist * 0.5;
        }

        menu(":regraph", grid, tree, last);

    } else if (name == ":help") {

        std::cout << "For equation help, see README.md\n"
                  << "Equation commands:\n"
                  << "    :help - Help\n"
                  << "    :solve :v - Solve the last equation for a value\n"
                  << "    :regraph - Graph the same equation again\n"
                  << "    :save :s - Save the last equation\n"
                  << "    :list :ls - Recall a saved equation\n"
                  << "    :recall :rs - Recall a saved equation (also lists saved equations)\n"
                  << "Graph window:\n"
                  << "    :zoomin :i - Zoom in (*2) in the center of the graph\n"
                  << "    :zoomout :o - Zoom out (*1/2) in the center of the graph\n"
                  << "    :l :r :u :d - Scroll left, right, up, down (*1/4)\n"
                  << "    :center :cg - Center graph at (0, 0)\n"
                  << "    :window - Show graph window position\n"
                  << "    :wedit - Edit graph window position\n"
                  << "Exit calculator:\n"
                  << "    :quit :q - Exit\n";

    } else if (name == ":quit" || name == ":q") {
        return true;
    } else {
        std::cout << "Unknown option\n";
    }

    return false;
}

int main() {
    bool verbose{ false };
    std::cout << "GRAPHING CALCULATOR v2\n"
              << "Enter an equation, or\n"
              << ":help for help\n";

    Grid userOptions{ };
    TokenArr tokenized{ };
    TreeItem tree{ .function="0" };
    std::string last{ "" };

    while (true) {
        std::string equation{ getLine("Enter an equation: 0 = ") };

        if (equation.starts_with(':')) {
            if (menu(equation, userOptions, tree, last)) break;
            continue;
        }
        // After menu(), so menu can see last
        last = equation;

        bool error{ false };
        std::cout << "(I) Tokenizing equation...\n";
        tokenized = tokenize(equation, error);

        std::cout << "(I) Cleaning equation...\n";
        if (error || !clean(tokenized)) {
            std::cout << "<!> [main:0] Parsing failed\n";
            continue;
        }

        if (verbose) {
            for (const Token& tk : tokenized) {
                printToken(tk);
            }
        }

        std::cout << "(I) Creating tree...\n";
        tree = buildTree(tokenized);

        if (verbose) printTree(tree);

        Grid g{ createGrid(tree, userOptions) };
        if (verbose) printGrid(g);
        drawGrid(g);

    }

    // std::regex regex{ "\\(" };
    // std::smatch match;
    // std::regex_search(equation, match, regex);
    // std::cout << "Match: " << match.str() << "\n";

    std::cout << "\nDone!\n";
    return 0;
}
