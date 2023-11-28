#include "deque.h"
#include "test_runner.h"
#include <iostream>
#include <deque>

using namespace std;

int main() {
    Deque<int> deque1;
    deque<int> deque2;

    for (int i = 0; i < 3; i++) {
        deque1.PushBack(i);
        deque2.push_back(i);
    }

    for (int i = 0; i < 3; i++) {
        cout << deque1[i] << endl;
        cout << deque2[i] << endl;
        cout << endl;
    }
}
