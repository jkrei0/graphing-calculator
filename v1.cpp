#include <iostream>
#include <vector>
#include <functional>
#include <algorithm>
#include <map>
#include <limits>
#include <string>
#include <cmath>

// change to print equation debug information
bool debugOutputEnabled{ false };

double distance(double a, double b) {
    return std::sqrt(std::pow(a-b, 2));
}
double distance(double ax, double ay, double bx, double by) {
    return std::sqrt(std::pow(ax-bx, 2) + std::pow(ay-by, 2));
}

// Clears anything waiting in the input buffer
void clearCin() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
// Has the user enter a string
std::string getString(std::string prompt) {
    std::string input{ };
    while (true) {
        // Prompt for input
        std::cout << prompt;
        std::cin >> input;

        // If failure, clear it and try again
        if (std::cin.fail()) {
            std::cin.clear();
            clearCin();

        // Else break and return the value
        } else { 
            clearCin();
            return input;
        }
        std::cout << "That's not valid. ";
    }
}
// Has the user enter a line of text
std::string getLine(std::string prompt) {
    std::string input{ };
    // Prompt for input
    std::cout << prompt;
    std::getline(std::cin >> std::ws, input); // ws to ignore leading whitespace.
    return input;
}
// std::to_string converts chars to ascii. This does it properly.
std::string charToString(char in) {
    std::string out{ in };
    return out;
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

namespace tkError {
    std::string unclosedParen{ "Unclosed parenthesis" };
    std::string unknownSymbol{ "Unknown symbol" };
    std::string setAlreadySolved{ "SET(0) at solved token" };
}
enum class tkType {
    number,
    operation,
    operationPriority,
    variable,
    group,
    undecided,
    solved
};
struct Token {
    tkType type;
    unsigned int position;
    std::string contents;
    double numerical{ 0 };
    std::vector<Token> tkContents;
    unsigned int index;
};

std::string tkAsString(const Token token) {
    std::string out{ std::to_string(token.position) + ": " };
    if (token.position < 10) {
        out = '0' + out;
    }
    switch (token.type) {
        case tkType::undecided: out += "U"; break;
        case tkType::number:    out += "N"; break;
        case tkType::operation: out += "O"; break;
        case tkType::operationPriority: out += "P"; break;
        case tkType::variable:  out += "V"; break;
        case tkType::group:     out += "G"; break;
        case tkType::solved:    out += "S"; break;
        default:                out += "?";
    }
    out += " = " + token.contents + " (" + std::to_string(token.numerical) + ")";
    return out;
}

std::vector<Token> tokenize(std::string function) {
    std::vector<Token> out{ };
    for (unsigned int i{ 0 }; i < function.size(); i++) {
        char character{ function[i] };
        // ignore whitespace
        if (std::isspace(character)) { continue; }

        // If it's a number
        if (std::isdigit(character)) {
            std::size_t digitLen{ 0 };
            double number{ std::stod(function.substr(i), &digitLen) };
            // Add new token
            Token newTk{ tkType::number, i, std::to_string(number), number };
            newTk.index = out.size();
            out.push_back(newTk);
            i += digitLen-1; // Skip digits used in the token (minus 1 because it'll be incremented again)

        } else if (character == '(') {
            Token newTk{ tkType::group, i, "" };
            // Find the end of the parentheses
            int parenCount{ 0 };
            for (int k{ 1 }; i+k < function.size(); k++) {
                if (function[i+k] == '(') {
                    parenCount++;
                } else if (function[i+k] == ')') {
                    parenCount--;
                    if (parenCount < 0) {
                        newTk.contents = function.substr(i+1, k-1); // minue 1 to not include the parenthesis
                        newTk.tkContents = tokenize(newTk.contents);
                        break;
                    }
                }
            }
            // if unclosed parentheses
            if (parenCount >= 0) {
                newTk.type = tkType::undecided;
                newTk.contents = tkError::unclosedParen;
            }
            newTk.index = out.size();
            out.push_back(newTk);
            i += newTk.contents.size() + 1; //Skip digits used in the token (plus 1 to also skip the closing parenthesis)

        } else {
            Token newTk{ tkType::undecided, i, charToString(character) };
            switch (character) {
                case '*':
                case '/':
                case '^': newTk.type = tkType::operationPriority; break;
                case '+':
                case '-': newTk.type = tkType::operation; break;
                case 'x':
                case 'y': newTk.type = tkType::variable; break;
            }
            newTk.index = out.size();
            out.push_back(newTk);
        }
    }
    out.shrink_to_fit();
    return out;
}

double findVar(const char key, const std::map<char, double>& variables) {
    const auto foundValue = variables.find(key);
    double value{ 0 };
    if (foundValue != variables.end()) {
        value = foundValue->second;
    }
    return value;
}

double solve(std::vector<Token> function, const std::map<char, double>& variables) {

    // Define i outsize the loop to use it in the lambda functions.
    int i{ 0 };
    int* iPtr{ &i };

    // Capture by pointer to be able to modify + read values.
    std::vector<Token>* funcPtr{ &function };
    std::function<Token&(int)> GET{ [iPtr, funcPtr](int x) -> Token& {
        int i{ *iPtr };
        if (i+x < 0) x = -i;
        if ((unsigned)(i+x) > funcPtr->size()) x = funcPtr->size() - i;
        while ((*funcPtr)[i+x].type == tkType::solved) {
            if (x < 0) { 
                if (i+x < 1) break; // if it's at the beginning, don't go over
                x -= 1;
            } else if (x > 0) { 
                if ((unsigned)(i+x) > funcPtr->size()-1) break; // if it's at the end, don't go over
                x += 1;
            } else break;
        }
        return (*funcPtr)[i+x];
    } };
    std::function<void(int, int)> MARK{ [iPtr, funcPtr](int x, int y) {
        int i{ *iPtr };
        for (int k{ i+x }; k <= i+y; k++) {
            // if the start/end of the range is a solved token, extend it until it's not.
            while ((*funcPtr)[k].type == tkType::solved) {
                if (k == i+x&& x < 0) { // if it's the start and going backwards
                    x -= 1;
                    k -= 1;
                } else if (k == i+y && x > 0) { // if it's the end and going forwards
                    y += 1;
                    k += 1;
                } else break; // if it's in the middle somewhere, ignore it.
            }
            (*funcPtr)[k].type = tkType::solved;
        }
    } };
    std::function<void(int, double)> SET{ [GET](int x, double k) {
        Token& selToken{ GET(x) };
        selToken.type = tkType::number;
        selToken.contents = std::to_string(k);
        selToken.numerical = k;
    } };
    std::function<double(int, char, int)> SOLVE{ [GET](int x, char oper, int y) -> double {
        double xVal{ GET(x).numerical };
        double yVal{ GET(y).numerical };
        switch (oper) {
            case '+': return xVal + yVal;
            case '-': return xVal - yVal;
            case '*': return xVal * yVal;
            case '/': return xVal / yVal;
            case '^': return std::pow(xVal, yVal);
            default:
                std::cout << "\nWARNING unsupported operation type " << oper << " (returning 0) (solve/SOLVE)\n";
                return 0;
        }
    } };
    std::function<bool(int)> ISVALUE{ [GET](int x) -> bool {
        Token& selToken{ GET(x) };
        return (selToken.type == tkType::number || selToken.type == tkType::variable);
    } };

    // first pass for () and replace variables.
    for (; (unsigned)i < function.size(); i++) {
        if (function[i].type == tkType::group) {
            SET(0, solve(function[i].tkContents, variables));
        } else if (function[i].type == tkType::variable) {
            function[i].numerical = findVar(function[i].contents[0], variables);
        }
    }
    i = 1; // reset i, next pass for ^
    while ((unsigned)i < function.size()) {
        Token& current{ GET(0) };
        Token& next{ GET(1) };

        if (current.type == tkType::operationPriority && current.contents == "^") { // eg x^2
            SET(1, SOLVE(-1, current.contents[0], 1));
            MARK(-1, 0);
            i = next.index;
        }
        i++;
    }
    i = 1; // reset i, next pass for * /
    while ((unsigned)i < function.size()) {
        Token& current{ GET(0) };
        Token& next{ GET(1) };

        if (ISVALUE(-1) && ISVALUE(0)) { // eg 3x
            SET(0, SOLVE(-1, '*', 0));
            MARK(-1, -1);
        } else if (current.type == tkType::operationPriority) { // eg 4 * 5
            SET(1, SOLVE(-1, current.contents[0], 1));
            MARK(-1, 0);
            i = next.index;
        }
        i++;
    }
    i = 0; // reset i, next pass for + -
    while ((unsigned)i < function.size()) {
        Token& current{ GET(0) };
        Token& next{ GET(1) };

        if (current.type == tkType::operation) { // eg 4 - 5
            SET(1, SOLVE(-1, current.contents[0], 1));
            MARK(-1, 0);
            i = next.index;
        }
        i++;
    }
    i = -1; // reset before GET;
    double toReturn{ GET(1).numerical };
    return toReturn;
}

void drawGraph(const std::vector<Token>& function, int start, int end, int min, int max) {
    std::vector<double> points{ };
    points.resize((end-start)*2);

    // Solve for all x-values
    for (int i{ 0 }; (unsigned)i < points.size(); i++) {
        double half{ static_cast<double>(i) };
        half /= 2;

        std::map<char, double> variables{ {'x', half + start} };
        points[i] = solve(function, variables);
    }
    
    for (int y{ max }; y >= min; y--) {

        for (unsigned int x{ 0 }; x < points.size(); x++) {
            if (std::round(points[x]) == y) { // draw the point
                std::cout << "o"; 
            } else {
                double current{ points[x] };
                double next{ current };
                double previous{ current };
                if (x > 0) previous = points[x-1];
                if (x < points.size()-1) next = points[x+1];

                // if between prev and current
                if (points[x] == INFINITY) {
                    std::cout << "'";
                } else if (next == INFINITY || previous == INFINITY) {
                    std::cout << " ";
                } else if ((y < previous && y > current) || ( (y > previous && y < current)
                    && y != std::round(next) && y != std::round(previous)
                    && (distance(x, y, next, x+1) > distance(x, y, previous, x-1)) )) {
                    std::cout << "x";

                } else if ((y < next && y > current) || ( (y > next && y < current)
                    && y != std::round(next) && y != std::round(previous)
                    && (distance(x, y, next, x+1) < distance(x, y, previous, x-1)) )) {
                    std::cout << "x";

                } else if (x == (unsigned)(-start*2)) { // draw y-axis
                    std::cout << "|"; 
                } else if (y == 0) { // draw x-axis
                    std::cout << "-";
                } else {
                    std::cout << " ";
                }
            }
        }
        std::cout << ' ' << y << '\n';

    }
}

/** 
 * Converts a function to an equation by encasing the function in a group and subtracting y
 */
std::vector<Token> functionToEquation(const std::vector<Token>& function) {
    std::vector<Token> copy{ };
    Token groupTk{ tkType::group, 0, "" };
    groupTk.tkContents = function;
    groupTk.index = 0;
    copy.push_back(groupTk);
    
    Token subtrTk{ tkType::operation, 1, "-" };
    subtrTk.index = 1;

    Token yVarTk{ tkType::variable, 2, "y" };
    yVarTk.index = 2;

    copy.push_back(subtrTk);
    copy.push_back(yVarTk);

    return copy;
}

std::vector<std::vector<bool>> plotEquation(const std::vector<Token>& function, int startx, int starty, int endx, int endy) {
    std::vector<std::vector<bool>> pointsGrid{ };
    // Start points must be BEFORE end points
    if (startx > endx || starty > endy) {
        return pointsGrid;
    }

    pointsGrid.resize(endy - starty);

    for (int y{ starty }; y < endy; y++) {
        std::vector<bool>& row{ pointsGrid.at(y - starty) };
        row.resize(endx - startx);

        for (int x{ startx }; x < endx; x++) {
            std::map<char, double> variables{ {'x', x}, {'y', y} };
            //std::cout << x - startx << ',' << y - starty << '\n';
            if (x - startx == y - starty) {
                std::string pl{ };
            }
            row.at(x - startx) = solve(function, variables ) >= 0;
            ;
            ;
        }
    }

    return pointsGrid;
}

/**
 * Draws from a 'pointsgrid' aka std::vector<std::vector<bool>>
 * Boolean pointsgrids only contain whether the number is positive or negative
 * Be aware: pointsgrids are laid out [y][x], for displaying/printing the graph more easily.
 * @param pointsGrid - Can be given from the plotEquation function that returns std::vector<std::vector<bool>>
 * @param startx - The starting (smallest) x value of the graph
 * @param starty - The starting (smallest) y value of the graph
 */
void drawPointsGrid(std::vector<std::vector<bool>>& pointsGrid, int startx, int starty) {

    for (int y{ static_cast<int>(pointsGrid.size()) - 2 }; y > 0; y--) {
        int curLine{ startx + y };

        std::vector<bool>& row{ pointsGrid.at(y) };

        for (unsigned int x{ 1 }; x < row.size() - 1; x++) {
            bool current = row.at(x);

            bool top{ pointsGrid.at(y + 1).at(x) };
            bool bottom{ pointsGrid.at(y - 1).at(x) };
            bool left{ row.at(x - 1) };
            bool right{ row.at(x + 1) };

            if ( debugOutputEnabled ) {

                /* Graphs positive/negative values instead of a line */
                if (current) {
                    if (x + startx == 0 || y + starty == 0) std::cout << "**";
                    else std::cout << "##";

                } else if (x + startx == 0 || y + starty == 0) {
                    std::cout << "++";
                } else std::cout << "  ";

            // Draw pixels on the border of - & + values, on the POSITIVE side of the border.
            // Because: zero is counted as positive .. and the line should be as close to 0 as possible
            // And only draw if the surrounding pixels aren't all positive
            } else if ( current && !(top && right && left && bottom) ) {
                std::cout << "88";
                
            } else if (x + startx == 0) {
                std::cout << "| ";
            } else if (curLine == 0) {
                std::cout << "--";

            } else {
                std::cout << "  ";
            }
        }
        std::cout << " : " << curLine << '\n';
    }
}

/**
 * @brief 
 * 
 * @param isFunction - If the user should be prompted for a function (f(x) = ) or an equation (0 = )
 */
void promptForEquation(bool isFunction) {
    // The first time, immediately start with entering an equation
    std::string input{ "new" };
    std::vector<Token> tokens;

    std::string equationName{ "equation" };
    std::string equationPrefix{ "0 = " };
    if (isFunction) {
        equationName = "function";
        equationPrefix = "f(x) = ";
    }

    while (input != "menu" && input != "m") {

        if (input == "new" || input == "n") {
            std::string function{ getLine("Enter " + equationName + ": " + equationPrefix) };
            tokens = tokenize(function);
        
            if (isFunction) {
                tokens = functionToEquation(tokens);
            }

            // Debug: Print a list of tokens
            if (debugOutputEnabled) {
                std::cout << "Tokenization summary: \n";
                for (unsigned int i{ 0 }; i < tokens.size(); i++) {
                    std::cout << tkAsString(tokens[i]) << '\n';
                    // Do children of only the first level
                    if (tokens[i].type == tkType::group) {
                        for (unsigned int k{ 0 }; k < tokens[i].tkContents.size(); k++) {
                            std::cout << "  - " << tkAsString(tokens[i].tkContents[k]) << '\n';
                        }
                    }
                }
            }

            std::cout << "\n ====== Graphed " << equationName << " ====== \n";

            auto pointsGrid{ plotEquation(tokens, -20, -20, 20, 20) };
            drawPointsGrid( pointsGrid, -20, -20 );

        } else if (input == "solve" || input == "s") {
            if (tokens.size() < 1) {
                std::cout << "There's no equation to solve!\n"
                    << "Use 'new' to enter an equation first.\n";
            } else {
                std::cout << "Solve equation for ..\n";
                double x{ getNumber("x value: ") };
                double y{ 0 };
                if (!isFunction) {
                    y = getNumber("y value: ");
                }

                std::map<char, double> variables{ {'x', x}, {'y', y} };
                std::cout << "Equation solved for x = " << x << ", y = " << y << ":\n"
                          << solve(tokens, variables) << "\n";
            }

        } else if (input == "help" || input == "h") {
            std::cout << "\nGraph Options:\n"
                << " new,   n  - graph a new " << equationName << '\n'
                << " solve, s  - solve this " << equationName << " given variables\n"
                << " help,  h  - show this list\n"
                << " menu,  m  - return to the menu\n";
        
        } else {
            std::cout << "Invalid option '" << input << "'. Please enter one of the following:";
        }

        input = getString("(Type 'h' for help) Enter an option: ");
    }
}

int main() {

    std::string input{ "" };

    std::cout << " ====== Welcome ======\n"
        << "This is a graphing calculator.\n"
        << "\nFor complete instructions, bug reports and source code see:\n"
        <<" https://github.com/jkrei0/graphing-calculator \n";

    std::cout << "\n(Type 'h' for help)\n";
    while (true) {
        input = getString("\nEnter an option: ");
        
        if (input == "exit" || input == "x") {
            break;

        } else if (input == "equation" || input == "e") {
            promptForEquation(false);
        } else if (input == "function" || input == "f") {
            promptForEquation(true);

        } else if (input == "debug" || input == "d") {
            debugOutputEnabled = !debugOutputEnabled;
            std::cout << "Debug output set to " << debugOutputEnabled << '\n';

        } else if (input == "help" || input == "h") {
            std::cout << "\nProgram Options:\n"
                << " function, f  - graph & solve functions\n"
                << " equation, e  - graph & solve equations\n"
                << " help,     h  - show this list\n"
                << " debug,    d  - toggle debug mode\n"
                << " exit,     x  - quit the program\n"
                << "\nFor complete instructions, and to report bugs, go to:\n"
                << " https://github.com/jkrei0/graphing-calculator\n";

        } else {
            std::cout << "Invalid option '" << input << "'. Please enter one of the following:";
        }
    
    }

    return 0;
}