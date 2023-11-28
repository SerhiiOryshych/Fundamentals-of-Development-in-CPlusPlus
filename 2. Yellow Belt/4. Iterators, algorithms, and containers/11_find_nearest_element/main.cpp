#include <set>
#include <iostream>

using namespace std;

set<int>::const_iterator FindNearestElement(
        const set<int> &numbers,
        int border) {
    auto it1 = numbers.lower_bound(border);
    auto it2 = it1 == numbers.begin() ? it1 : prev(it1);

    if (it2 == numbers.end()) {
        return it1;
    }

    if (it1 == numbers.end()) {
        return it2;
    }

    int x = *it1;
    int y = *it2;
    if (border - y <= x - border) {
        return it2;
    } else {
        return it1;
    }
}

int main() {
    set<int> numbers = {1, 4, 6};
    cout <<
         *FindNearestElement(numbers, 0) << " " <<
         *FindNearestElement(numbers, 3) << " " <<
         *FindNearestElement(numbers, 5) << " " <<
         *FindNearestElement(numbers, 6) << " " <<
         *FindNearestElement(numbers, 100) << endl;

    set<int> empty_set;

    cout << (FindNearestElement(empty_set, 8) == end(empty_set)) << endl;
    return 0;
}
