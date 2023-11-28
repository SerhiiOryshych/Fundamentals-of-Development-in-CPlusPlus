#include <iostream>
#include <sstream>
#include <vector>
#include <utility>
#include <cmath>
#include <iomanip>
#include <memory>

using namespace std;

class Figure {
public:
    [[nodiscard]] virtual string Name() const = 0;

    [[nodiscard]] virtual double Perimeter() const = 0;

    [[nodiscard]] virtual double Area() const = 0;
};

class Triangle : public Figure {
public:
    Triangle(double a, double b, double c) : a_(a), b_(b), c_(c) {}

    [[nodiscard]] string Name() const override {
        return "TRIANGLE";
    }

    [[nodiscard]] double Perimeter() const override {
        return a_ + b_ + c_;
    }

    [[nodiscard]] double Area() const override {
        double s_2 = Perimeter() / 2;
        return ::sqrt(s_2 * (s_2 - a_) * (s_2 - b_) * (s_2 - c_));
    }

private:
    const double a_, b_, c_;
};

class Rect : public Figure {
public:
    Rect(double a, double b) : a_(a), b_(b) {}

    [[nodiscard]] string Name() const override {
        return "RECT";
    }

    [[nodiscard]] double Perimeter() const override {
        return 2 * (a_ + b_);
    }

    [[nodiscard]] double Area() const override {
        return a_ * b_;
    }

private:
    const double a_, b_;
};

class Circle : public Figure {
public:
    explicit Circle(double r) : r_(r) {}

    [[nodiscard]] string Name() const override {
        return "CIRCLE";
    }

    [[nodiscard]] double Perimeter() const override {
        return r_ * 3.14 * 2;
    }

    [[nodiscard]] double Area() const override {
        return r_ * r_ * 3.14;
    }

private:
    const double r_;
};

shared_ptr<Figure> CreateFigure(istringstream &is) {
    string name;
    is >> name;
    if (name == "RECT") {
        int a, b;
        is >> a >> b;
        return make_shared<Rect>(a, b);
    } else if (name == "TRIANGLE") {
        int a, b, c;
        is >> a >> b >> c;
        return make_shared<Triangle>(a, b, c);
    } else {
        int r;
        is >> r;
        return make_shared<Circle>(r);
    }
}

int main() {
    vector<shared_ptr<Figure>> figures;
    for (string line; getline(cin, line);) {
        istringstream is(line);

        string command;
        is >> command;
        if (command == "ADD") {
            figures.push_back(CreateFigure(is));
        } else if (command == "PRINT") {
            for (const auto &current_figure: figures) {
                cout << fixed << setprecision(3)
                     << current_figure->Name() << " "
                     << current_figure->Perimeter() << " "
                     << current_figure->Area() << endl;
            }
        }
    }
    return 0;
}