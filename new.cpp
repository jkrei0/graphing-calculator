#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <regex>
#include <stdexcept>
#include <map>
#include <cmath>
#include <limits>
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

#include "calculator/def.hpp"
#include "calculator/tokens.hpp"
#include "calculator/tree.hpp"
#include "calculator/solve.hpp"
#include "calculator/grid.hpp"

// Menu commands. Returns true on :quit, false otherwise
bool menu(std::string query, Grid& grid, TreeItem& tree, std::string last, double arg = 0) {
    struct Save{
        std::string name;
        TreeItem tree;
    };
    static std::vector<Save> savedEquations{ };

    const std::regex rQueryName{ "^:[a-z]+" };
    std::smatch match;
    if (!std::regex_search(query, match, rQueryName)) {
        std::cout << "Badly formatted query.\n";
        return false;
    }
    std::string name{ match.str() };

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
        if (tree.function == "0") {
            std::cout << "No equation to graph.\n";
            return false;
        }
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

        menu(":regraph", grid, recalled.tree, recalled.name);

    } else if (name == ":zi") {

        menu(":zoom", grid, tree, last, /* Arg */ 0.5);

    } else if (name == ":zo") {

        menu(":zoom", grid, tree, last, /* Arg */ 2);

    } else if (name == ":zoom" || name == ":z") {

        double level{ arg };
        while (level == 0) {
            level = std::abs(getNumber("Enter zoom level: "));
        }

        double two{2}; // b/c stupid int division
        double xDist{ (grid.endX - grid.startX) / two };
        double centerX{ xDist + grid.startX };
        double yDist{ (grid.endY - grid.startY) / two };
        double centerY{ yDist + grid.startY };
        
        double stepRatio{ grid.stepY / grid.stepX };

        grid.startX = centerX - (xDist*level);
        grid.endX = centerX + (xDist*level);
        grid.stepX *= level;

        grid.startY = centerY - (yDist*level);
        grid.endY = centerY + (yDist*level);
        grid.stepY = grid.stepX*stepRatio;

        std::cout << level << "l, " << stepRatio << "r \n";
        menu(":window", grid, tree, last);

        if (grid.startX >= grid.endX || grid.startY >= grid.endY) {
            grid.endX += 1;
            grid.endY += 1;
            std::cout << "Too small of a zoom factor!\n";
        }

        menu(":regraph", grid, tree, last);

    } else if (name == ":center" || name == ":c") {

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
                  << "    :zoom :z - Zoom to a specified amount at the center of the graph\n"
                  << "    :zi - Zoom in (*2) in the center of the graph\n"
                  << "    :zo - Zoom out (*0.5) in the center of the graph\n"
                  << "    :l :r :u :d - Scroll left, right, up, down (*1/4)\n"
                  << "    :center :c - Center graph at (0, 0)\n"
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
