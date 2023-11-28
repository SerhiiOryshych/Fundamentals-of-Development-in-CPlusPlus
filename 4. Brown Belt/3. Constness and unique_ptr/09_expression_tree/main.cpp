#include "Common.h"
#include "test_runner.h"

#include <sstream>

using namespace std;

class ValueNode : public Expression {
public:
    explicit ValueNode(int value) : value(value) {}

    [[nodiscard]] int Evaluate() const override {
        return value;
    }

    [[nodiscard]] string ToString() const override {
        return to_string(value);
    }

private:
    int value;
};

class BinaryOperationNode : public Expression {
public:
    BinaryOperationNode(ExpressionPtr left, ExpressionPtr right) : left(move(left)), right(move(right)) {}

    [[nodiscard]] string ToString(const char op) const {
        return "(" + left->ToString() + ")" + op + "(" + right->ToString() + ")";
    }

protected:
    ExpressionPtr left, right;
};

class SumNode : public BinaryOperationNode {
public:
    SumNode(ExpressionPtr left, ExpressionPtr right) : BinaryOperationNode(move(left), move(right)) {}

    [[nodiscard]] int Evaluate() const override {
        return left->Evaluate() + right->Evaluate();
    }

    [[nodiscard]] string ToString() const override {
        return BinaryOperationNode::ToString(op);
    }

private:
    const char op = '+';
};

class ProductNode : public BinaryOperationNode {
public:
    ProductNode(ExpressionPtr left, ExpressionPtr right) : BinaryOperationNode(move(left), move(right)) {}

    [[nodiscard]] int Evaluate() const override {
        return left->Evaluate() * right->Evaluate();
    }

    [[nodiscard]] string ToString() const override {
        return BinaryOperationNode::ToString(op);
    }

private:
    const char op = '*';
};

ExpressionPtr Value(int value) {
    return make_unique<ValueNode>(value);
}

ExpressionPtr Sum(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<SumNode>(move(left), move(right));
}

ExpressionPtr Product(ExpressionPtr left, ExpressionPtr right) {
    return make_unique<ProductNode>(move(left), move(right));
}

string Print(const Expression *e) {
    if (!e) {
        return "Null expression provided";
    }
    stringstream output;
    output << e->ToString() << " = " << e->Evaluate();
    return output.str();
}

void Test() {
    ExpressionPtr e1 = Product(Value(2), Sum(Value(3), Value(4)));
    ASSERT_EQUAL(Print(e1.get()), "(2)*((3)+(4)) = 14");

    ExpressionPtr e2 = Sum(move(e1), Value(5));
    ASSERT_EQUAL(Print(e2.get()), "((2)*((3)+(4)))+(5) = 19");

    ASSERT_EQUAL(Print(e1.get()), "Null expression provided");
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, Test);
    return 0;
}
