#include <iostream>
#include <map>
#include <string>

using namespace std;

template<typename Key, typename Value>
Value &GetRefStrict(map<Key, Value> &m, const Key &key) {
    auto it = m.find(key);
    if (it == m.end()) {
        throw runtime_error("");
    }
    return it->second;
}

int main() {
    map<int, string> m = {{0, "value"}};
    string &item = GetRefStrict(m, 0);
    item = "newvalue";
    cout << m[0] << endl; // newvalue
}
