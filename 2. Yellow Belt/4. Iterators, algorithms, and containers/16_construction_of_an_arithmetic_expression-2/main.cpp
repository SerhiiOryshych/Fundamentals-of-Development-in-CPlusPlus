#include <iostream>
#include <deque>

using namespace std;

int main() {
    int x;
    cin >> x;

    deque<string> deq{to_string(x)};

    int n;
    cin >> n;
    string prev_operation;
    while (n--) {
        string operation;
        int value;
        cin >> operation >> value;
        if (operation == "*" || operation == "/") {
            if (prev_operation == "+" || prev_operation == "-") {
                deq.emplace_front("(");
                deq.emplace_back(")");
            }
        }
        deq.push_back(" " + operation + " " + to_string(value));
        prev_operation = operation;
    }

    for (const auto &deq_value: deq) {
        cout << deq_value;
    }
    cout << endl;
}
