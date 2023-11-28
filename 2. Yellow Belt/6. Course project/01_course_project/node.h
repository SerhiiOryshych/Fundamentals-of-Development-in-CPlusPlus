#pragma once

#include "date.h"
#include <memory>
#include <utility>

using namespace std;

enum class Comparison {
    Less,
    LessOrEqual,
    Greater,
    GreaterOrEqual,
    Equal,
    NotEqual
};

enum struct LogicalOperation {
    Or,
    And
};

class Node {
public:
    [[nodiscard]] virtual bool Evaluate(const Date &date, const string &event) const = 0;
};

class EmptyNode : public Node {
public:
    EmptyNode();

    [[nodiscard]] bool Evaluate(const Date &date, const string &event) const override;
};

class DateComparisonNode : public Node {
public:
    DateComparisonNode(const Comparison &cmp, const Date &date);

    [[nodiscard]] bool Evaluate(const Date &date, const string &event) const override;

private:
    const Comparison cmp_;
    const Date date_;
};

class EventComparisonNode : public Node {
public:
    EventComparisonNode(const Comparison &cmp, string event);

    [[nodiscard]] bool Evaluate(const Date &date, const string &event) const override;

private:
    const Comparison cmp_;
    const string event_;
};

class LogicalOperationNode : public Node {
public:
    LogicalOperationNode(const LogicalOperation &logical_operation, const shared_ptr<Node> &left,
                         const shared_ptr<Node> &right);

    [[nodiscard]] bool Evaluate(const Date &date, const string &event) const override;

private:
    const LogicalOperation logical_operation_;
    const shared_ptr<Node> left_, right_;
};