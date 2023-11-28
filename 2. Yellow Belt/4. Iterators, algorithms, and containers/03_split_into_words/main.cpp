#include <vector>
#include <iostream>

using namespace std;

vector<string> SplitIntoWords(const string &s) {
    vector<string> response;

    auto it = s.begin();
    while (it != s.end()) {
        auto next_space = find(it, s.end(), ' ');

        if (it < next_space) {
            response.emplace_back(it, next_space);
        }

        if (next_space != s.end()) {
            next_space++;
        }
        it = next_space;
    }

    return response;
}

int main() {
    string s = "C    Cpp Java Python";

    vector<string> words = SplitIntoWords(s);
    cout << words.size() << " ";
    for (auto it = begin(words); it != end(words); ++it) {
        if (it != begin(words)) {
            cout << "/";
        }
        cout << *it;
    }
    cout << endl;

    return 0;
}
