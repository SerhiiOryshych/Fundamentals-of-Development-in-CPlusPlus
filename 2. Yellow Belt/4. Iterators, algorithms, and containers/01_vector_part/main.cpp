#include <iostream>
#include <vector>

using namespace std;

void PrintVectorPart(const vector<int> &numbers) {
    auto firstNegative = find_if(numbers.begin(), numbers.end(), [](int x) { return x < 0; });
    while (firstNegative != numbers.begin()) {
        firstNegative--;
        cout << *firstNegative << " ";
    }
}

int main() {
    PrintVectorPart({6, 1, 8, -5, 4});
    cout << endl;
    PrintVectorPart({-6, 1, 8, -5, 4});
    cout << endl;
    PrintVectorPart({6, 1, 8, 5, 4});
    cout << endl;
    return 0;
}
