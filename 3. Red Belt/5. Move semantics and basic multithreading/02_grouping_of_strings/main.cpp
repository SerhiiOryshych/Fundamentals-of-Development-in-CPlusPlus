#include "test_runner.h"

#include <algorithm>
#include <string>
#include <vector>
#include <set>

using namespace std;

template<typename String>
using Group = vector<String>;

template<typename String>
using Char = typename String::value_type;

template<typename String>
vector<Group<String>> GroupHeavyStrings(vector<String> strings) {
    if (strings.empty()) {
        return {};
    }

    vector<pair<String, int>> string_with_index;
    vector<set<Char<String>>> index_to_set;
    for (int i = 0; i < strings.size(); i++) {
        index_to_set.push_back({});
        for (const auto &ch: strings[i]) {
            index_to_set[i].insert(ch);
        }
        string_with_index.push_back({String(), i});
        string_with_index.back().first = std::move(strings[i]);
    }

    stable_sort(begin(string_with_index), end(string_with_index),
                [&index_to_set](const pair<String, int> &s1, const pair<String, int> &s2) {
                    return index_to_set[s1.second] < index_to_set[s2.second];
                });

    vector<Group<String>> result = {{String()}};
    result.back().back() = std::move(string_with_index[0].first);

    for (int i = 1; i < string_with_index.size(); i++) {
        if (index_to_set[string_with_index[i - 1].second] == index_to_set[string_with_index[i].second]) {
            result.back().push_back(String());
        } else {
            result.push_back({String()});
        }
        result.back().back() = std::move(string_with_index[i].first);
    }
    return result;
}


void TestGroupingABC() {
    vector<string> strings = {"caab", "abc", "cccc", "bacc", "c"};
    auto groups = GroupHeavyStrings(strings);
    ASSERT_EQUAL(groups.size(), 2);
    sort(begin(groups), end(groups));  // The order of groups does not matter.
    ASSERT_EQUAL(groups[0], vector<string>({"caab", "abc", "bacc"}));
    ASSERT_EQUAL(groups[1], vector<string>({"cccc", "c"}));
}

void TestGroupingReal() {
    vector<string> strings = {"law", "port", "top", "laptop", "pot", "paloalto", "wall", "awl"};
    auto groups = GroupHeavyStrings(strings);
    ASSERT_EQUAL(groups.size(), 4);
    sort(begin(groups), end(groups));  // The order of groups does not matter.
    ASSERT_EQUAL(groups[0], vector<string>({"laptop", "paloalto"}));
    ASSERT_EQUAL(groups[1], vector<string>({"law", "wall", "awl"}));
    ASSERT_EQUAL(groups[2], vector<string>({"port"}));
    ASSERT_EQUAL(groups[3], vector<string>({"top", "pot"}));
}

int main() {
    TestRunner tr;
    RUN_TEST(tr, TestGroupingABC);
    RUN_TEST(tr, TestGroupingReal);
    return 0;
}
