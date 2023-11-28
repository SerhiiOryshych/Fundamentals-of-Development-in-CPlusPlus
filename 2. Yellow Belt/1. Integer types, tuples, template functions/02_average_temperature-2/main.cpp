#include <iostream>
#include <vector>

using namespace std;

int main() {
    int n;
    cin >> n;

    long long sum = 0;
    vector<int> temps(n);
    for (int i = 0; i < n; i++) {
        cin >> temps[i];
        sum += temps[i];
    }
    
    long long avg = sum / n;

    vector<int> daysWithTempGreaterThanAvg;
    for (int i = 0; i < n; i++) {
        if (temps[i] > avg) {
            daysWithTempGreaterThanAvg.push_back(i);
        }
    }

    cout << daysWithTempGreaterThanAvg.size() << endl;
    for (auto x: daysWithTempGreaterThanAvg) {
        cout << x << " ";
    }
    cout << endl;
}
