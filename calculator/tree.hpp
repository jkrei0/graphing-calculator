#pragma once
#include "tokens.hpp"

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
