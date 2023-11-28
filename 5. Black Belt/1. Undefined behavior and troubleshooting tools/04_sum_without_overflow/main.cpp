#include <iostream>
#include <cstdint>
#include <limits>

using namespace std;

int main() {
    int64_t a, b;
    if (cin >> a >> b) {
        int64_t max_value = numeric_limits<int64_t>::max();
        int64_t min_value = numeric_limits<int64_t>::min();

        if ((a > 0 && b > max_value - a) || (a < 0 && b < min_value - a)) {
            cout << "Overflow!" << endl;
        } else {
            int64_t sum = a + b;
            cout << sum << endl;
        }
    } else {
        cout << "Invalid input!" << endl;
    }

    return 0;
}
