#include "numeric.h"

#include <cmath>

using namespace std;

static constexpr double EPS = 1e-6;

bool IsZero(double x) {
    return abs(x) < EPS;
}

int CeilToInt(double x) {
    return static_cast<int>(ceil(x) + EPS);
}

double Modulo(double value, int modulo) {
    if (value < 0) {
        value += CeilToInt(-value / modulo) * modulo;
    }
    const auto value_int = static_cast<int>(value);
    return value_int % modulo + (value - value_int);
}
