#pragma once

#include <memory>
#include <string>

// Base class for an arithmetic expression.
class Expression {
public:
    virtual ~Expression() = default;

    // Calculates the value of the expression.
    virtual int Evaluate() const = 0;

    // Formats the expression as a string.
    // Each node is enclosed in parentheses, regardless of priority.
    virtual std::string ToString() const = 0;
};

using ExpressionPtr = std::unique_ptr<Expression>;

// Functions for forming the expression.
ExpressionPtr Value(int value);

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right);

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right);
