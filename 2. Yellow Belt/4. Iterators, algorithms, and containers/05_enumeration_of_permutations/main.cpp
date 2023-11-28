#include <vector>
#include <iostream>
#include <numeric>

using namespace std;

int main() {
    int n;
    cin >> n;
    vector<int> p(n);
    iota(p.rbegin(), p.rend(), 1);

    auto printVector = [](const vector<int> &v) {
        for (auto const x: v) {
            cout << x << " ";
        }
        cout << endl;
    };

    do {
        printVector(p);
    } while (prev_permutation(p.begin(), p.end()));
}
