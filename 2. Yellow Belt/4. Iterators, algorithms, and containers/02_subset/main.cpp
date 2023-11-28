#include <iostream>
#include <vector>
#include <set>

using namespace std;

template<typename T>
vector<T> FindGreaterElements(const set<T> &elements, const T &border) {
    vector<T> response;
    for_each(elements.begin(), elements.end(),
             [border, &response](const T &x) { if (x > border) response.push_back(x); });
    return response;
}

int main() {
    for (int x: FindGreaterElements(set<int>{1, 5, 7, 8}, 5)) {
        cout << x << " ";
    }
    cout << endl;

    string to_find = "Python";
    cout << FindGreaterElements(set<string>{"C", "C++"}, to_find).size() << endl;
    return 0;
}
