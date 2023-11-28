#include <iostream>

using namespace std;

int main() {
    int n, r;
    cin >> n >> r;

    unsigned long long sum = 0;
    for (int i = 0; i < n; i++) {
        int w, h, d;
        cin >> w >> h >> d;
        sum += r * w * h * d;
    }

    cout << sum << endl;
}
