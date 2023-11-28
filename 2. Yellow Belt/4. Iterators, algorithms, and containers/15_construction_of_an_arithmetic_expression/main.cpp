#include <iostream>
#include <deque>

using namespace std;

int main() {
    int x;
    cin >> x;

    deque<string> deq{to_string(x)};

    int n;
    cin >> n;
    while (n--) {
        string q;
        int value;
        cin >> q >> value;
        deq.emplace_front("(");
        deq.push_back(") " + q + " " + to_string(value));
    }

    for (const auto &deq_value: deq) {
        cout << deq_value;
    }
    cout << endl;
}
