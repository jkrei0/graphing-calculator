#pragma once
#include "solve.hpp"
#include <iomanip>

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
// IMPORTANT: The grid's settings must actually reflect the dimensions of the vectors!
void drawGrid(const Grid& grid, bool thick = false) {
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
            bool sTop{ (grid.points.at(x).at(y+1) >= 0) != sign };
            bool sBottom{ (grid.points.at(x).at(y-1) >= 0) != sign };
            bool sRight{ (grid.points.at(x+1).at(y) >= 0) != sign };
            bool sLeft{ (grid.points.at(x-1).at(y) >= 0) != sign };

            // easy points
            if (grid.points.at(x).at(y) == 0) std::cout << "0";
            // positive side
            else if (sign && (sTop || sBottom || sRight || sLeft)) std::cout << "O";
            // negative side only if "thick"
            else if (!sign && (sTop || sBottom || sRight || sLeft) && thick) std::cout << "*";
            else if (std::abs(actualX) < grid.stepX/2) std::cout << '|';
            else if (std::abs(actualY) < grid.stepY/2) std::cout << '_';
            else std::cout << ' ';
        }
        std::cout << '\n';
    }
    std::cout << "=====\n";
}
